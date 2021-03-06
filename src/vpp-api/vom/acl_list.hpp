/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __VOM_ACL_LIST_H__
#define __VOM_ACL_LIST_H__

#include <set>

#include "vom/acl_l2_rule.hpp"
#include "vom/acl_l3_rule.hpp"
#include "vom/acl_types.hpp"
#include "vom/dump_cmd.hpp"
#include "vom/hw.hpp"
#include "vom/inspect.hpp"
#include "vom/om.hpp"
#include "vom/rpc_cmd.hpp"
#include "vom/singular_db.hpp"

#include <vapi/acl.api.vapi.hpp>

namespace VOM {
namespace ACL {
/**
 * An ACL list comprises a set of match actions rules to be applied to
 * packets.
 * A list is bound to a given interface.
 */
template <typename RULE, typename UPDATE, typename DELETE, typename DUMP>
class list : public object_base
{
public:
  /**
   * The KEY can be used to uniquely identify the ACL.
   * (other choices for keys, like the summation of the properties
   * of the rules, are rather too cumbersome to use
   */
  typedef std::string key_t;

  /**
   * The rule container type
   */
  typedef std::multiset<RULE> rules_t;

  /**
   * Construct a new object matching the desried state
   */
  list(const key_t& key)
    : m_key(key)
  {
  }

  list(const handle_t& hdl, const key_t& key)
    : m_hdl(hdl)
    , m_key(key)
  {
  }

  list(const key_t& key, const rules_t& rules)
    : m_key(key)
    , m_rules(rules)
  {
    m_evh.order();
  }

  /**
   * Copy Constructor
   */
  list(const list& o)
    : m_hdl(o.m_hdl)
    , m_key(o.m_key)
    , m_rules(o.m_rules)
  {
  }

  /**
   * Destructor
   */
  ~list()
  {
    sweep();
    m_db.release(m_key, this);
  }

  /**
   * Return the 'sigular instance' of the ACL that matches this object
   */
  std::shared_ptr<list> singular() const { return find_or_add(*this); }

  /**
   * Dump all ACLs into the stream provided
   */
  static void dump(std::ostream& os) { m_db.dump(os); }

  /**
   * convert to string format for debug purposes
   */
  std::string to_string() const
  {
    std::ostringstream s;
    s << "acl-list:[" << m_key << " " << m_hdl.to_string() << " rules:[";

    for (auto rule : m_rules) {
      s << rule.to_string() << " ";
    }

    s << "]]";

    return (s.str());
  }

  /**
   * Insert priority sorted a rule into the list
   */
  void insert(const RULE& rule) { m_rules.insert(rule); }

  /**
   * Remove a rule from the list
   */
  void remove(const RULE& rule) { m_rules.erase(rule); }

  /**
   * Return the VPP assign handle
   */
  const handle_t& handle() const { return m_hdl.data(); }

  /**
   * A command class that Create the list
   */
  class update_cmd
    : public rpc_cmd<HW::item<handle_t>, HW::item<handle_t>, UPDATE>
  {
  public:
    /**
     * Constructor
     */
    update_cmd(HW::item<handle_t>& item, const key_t& key, const rules_t& rules)
      : rpc_cmd<HW::item<handle_t>, HW::item<handle_t>, UPDATE>(item)
      , m_key(key)
      , m_rules(rules)
    {
    }

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);

    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const
    {
      std::ostringstream s;
      s << "ACL-list-update: " << this->item().to_string();

      return (s.str());
    }

    /**
     * Comparison operator - only used for UT
     */
    bool operator==(const update_cmd& other) const
    {
      return ((m_key == other.m_key) && (m_rules == other.m_rules));
    }

    void complete()
    {
      std::shared_ptr<list> sp = find(m_key);
      if (sp && this->item()) {
        list::add(this->item().data(), sp);
      }
    }

    void succeeded()
    {
      rpc_cmd<HW::item<handle_t>, HW::item<handle_t>, UPDATE>::succeeded();
      complete();
    }

    /**
     * A callback function for handling ACL creates
     */
    virtual vapi_error_e operator()(UPDATE& reply)
    {
      int acl_index = reply.get_response().get_payload().acl_index;
      int retval = reply.get_response().get_payload().retval;

      VOM_LOG(log_level_t::DEBUG) << this->to_string() << " " << retval;

      HW::item<handle_t> res(acl_index, rc_t::from_vpp_retval(retval));

      this->fulfill(res);

      return (VAPI_OK);
    }

  private:
    /**
     * The key.
     */
    const key_t& m_key;

    /**
     * The rules
     */
    const rules_t& m_rules;
  };

  /**
   * A cmd class that Deletes an ACL
   */
  class delete_cmd : public rpc_cmd<HW::item<handle_t>, rc_t, DELETE>
  {
  public:
    /**
     * Constructor
     */
    delete_cmd(HW::item<handle_t>& item)
      : rpc_cmd<HW::item<handle_t>, rc_t, DELETE>(item)
    {
    }

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con) { return (rc_t::INVALID); }

    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const
    {
      std::ostringstream s;
      s << "ACL-list-delete: " << this->item().to_string();

      return (s.str());
    }

    /**
     * Comparison operator - only used for UT
     */
    bool operator==(const delete_cmd& other) const
    {
      return (this->item().data() == other.item().data());
    }
  };

  /**
   * A cmd class that Dumps all the ACLs
   */
  class dump_cmd : public VOM::dump_cmd<DUMP>
  {
  public:
    /**
     * Constructor
     */
    dump_cmd() = default;
    dump_cmd(const dump_cmd& d) = default;

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con) { return rc_t::INVALID; }

    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const { return ("acl-list-dump"); }

  private:
    /**
     * HW reutrn code
     */
    HW::item<bool> item;
  };

  static std::shared_ptr<list> find(const handle_t& handle)
  {
    return (m_hdl_db[handle].lock());
  }

  static std::shared_ptr<list> find(const key_t& key)
  {
    return (m_db.find(key));
  }

  static void add(const handle_t& handle, std::shared_ptr<list> sp)
  {
    m_hdl_db[handle] = sp;
  }

  static void remove(const handle_t& handle) { m_hdl_db.erase(handle); }

private:
  /**
   * Class definition for listeners to OM events
   */
  class event_handler : public OM::listener, public inspect::command_handler
  {
  public:
    event_handler()
    {
      OM::register_listener(this);
      inspect::register_handler({ "acl" }, "ACL lists", this);
    }
    virtual ~event_handler() = default;

    /**
     * Handle a populate event
     */
    void handle_populate(const client_db::key_t& key);

    /**
     * Handle a replay event
     */
    void handle_replay() { m_db.replay(); }

    /**
     * Show the object in the Singular DB
     */
    void show(std::ostream& os) { m_db.dump(os); }

    /**
     * Get the sortable Id of the listener
     */
    dependency_t order() const { return (dependency_t::ACL); }
  };

  /**
   * event_handler to register with OM
   */
  static event_handler m_evh;

  /**
   * Enquue commonds to the VPP command Q for the update
   */
  void update(const list& obj)
  {
    /*
     * always update the instance with the latest rule set
     */
    if (!m_hdl || obj.m_rules != m_rules) {
      HW::enqueue(new update_cmd(m_hdl, m_key, m_rules));
    }
    /*
     * We don't, can't, read the priority from VPP,
     * so the is equals check above does not include the priorty.
     * but we save it now.
     */
    m_rules = obj.m_rules;
  }

  /**
   * HW assigned handle
   */
  HW::item<handle_t> m_hdl;

  /**
   * Find or add the sigular instance in the DB
   */
  static std::shared_ptr<list> find_or_add(const list& temp)
  {
    return (m_db.find_or_add(temp.m_key, temp));
  }

  /*
   * It's the VOM::OM class that updates call update
   */
  friend class VOM::OM;

  /**
   * It's the VOM::singular_db class that calls replay()
   */
  friend class singular_db<key_t, list>;

  /**
   * Sweep/reap the object if still stale
   */
  void sweep(void)
  {
    if (m_hdl) {
      HW::enqueue(new delete_cmd(m_hdl));
    }
    HW::write();
  }

  /**
   * Replay the objects state to HW
   */
  void replay(void)
  {
    if (m_hdl) {
      HW::enqueue(new update_cmd(m_hdl, m_key, m_rules));
    }
  }

  /**
   * A map of all ACL's against the client's key
   */
  static singular_db<key_t, list> m_db;

  /**
   * A map of all ACLs keyed against VPP's handle
   */
  static std::map<const handle_t, std::weak_ptr<list>> m_hdl_db;

  /**
   * The Key is a user defined identifer for this ACL
   */
  const key_t m_key;

  /**
   * A sorted list of the rules
   */
  rules_t m_rules;
};

/**
 * Typedef the L3 ACL type
 */
typedef list<l3_rule, vapi::Acl_add_replace, vapi::Acl_del, vapi::Acl_dump>
  l3_list;

/**
 * Typedef the L2 ACL type
 */
typedef list<l2_rule,
             vapi::Macip_acl_add,
             vapi::Macip_acl_del,
             vapi::Macip_acl_dump>
  l2_list;

/**
 * Definition of the static singular_db for ACL Lists
 */
template <typename RULE, typename UPDATE, typename DELETE, typename DUMP>
singular_db<typename ACL::list<RULE, UPDATE, DELETE, DUMP>::key_t,
            ACL::list<RULE, UPDATE, DELETE, DUMP>>
  list<RULE, UPDATE, DELETE, DUMP>::m_db;

/**
 * Definition of the static per-handle DB for ACL Lists
 */
template <typename RULE, typename UPDATE, typename DELETE, typename DUMP>
std::map<const handle_t, std::weak_ptr<ACL::list<RULE, UPDATE, DELETE, DUMP>>>
  list<RULE, UPDATE, DELETE, DUMP>::m_hdl_db;

template <typename RULE, typename UPDATE, typename DELETE, typename DUMP>
typename ACL::list<RULE, UPDATE, DELETE, DUMP>::event_handler
  list<RULE, UPDATE, DELETE, DUMP>::m_evh;
};
};

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "mozilla")
 * End:
 */

#endif
