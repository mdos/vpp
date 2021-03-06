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

#ifndef __VOM_BRIDGE_DOMAIN_ENTRY_H__
#define __VOM_BRIDGE_DOMAIN_ENTRY_H__

#include "vom/bridge_domain.hpp"
#include "vom/interface.hpp"
#include "vom/singular_db.hpp"

#include <vapi/l2.api.vapi.hpp>

namespace VOM {
/**
 * A MAC forwarding entry in the bridge-domain/L2-FIB
 */
class bridge_domain_entry : public object_base
{
public:
  /**
   * The key for a bridge_domain
   */
  typedef std::pair<uint32_t, mac_address_t> key_t;

  /**
   * Construct a bridge_domain in the given bridge domain
   */
  bridge_domain_entry(const bridge_domain& bd,
                      const mac_address_t& mac,
                      const interface& tx_itf);

  /**
   * Construct a bridge_domain in the default table
   */
  bridge_domain_entry(const mac_address_t& mac, const interface& tx_itf);

  /**
   * Copy Construct
   */
  bridge_domain_entry(const bridge_domain_entry& r);

  /**
   * Destructor
   */
  ~bridge_domain_entry();

  /**
   * Return the matching 'singular instance'
   */
  std::shared_ptr<bridge_domain_entry> singular() const;

  /**
   * Find the instnace of the bridge_domain domain in the OM
   */
  static std::shared_ptr<bridge_domain_entry> find(
    const bridge_domain_entry& temp);

  /**
   * Dump all bridge_domain-doamin into the stream provided
   */
  static void dump(std::ostream& os);

  /**
   * replay the object to create it in hardware
   */
  void replay(void);

  /**
   * Convert to string for debugging
   */
  std::string to_string() const;

  /**
   * A command class that creates or updates the bridge_domain
   */
  class create_cmd : public rpc_cmd<HW::item<bool>, rc_t, vapi::L2fib_add_del>
  {
  public:
    /**
     * Constructor
     */
    create_cmd(HW::item<bool>& item,
               const mac_address_t& mac,
               uint32_t id,
               handle_t tx_intf);

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);

    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const;

    /**
     * Comparison operator - only used for UT
     */
    bool operator==(const create_cmd& i) const;

  private:
    mac_address_t m_mac;
    uint32_t m_bd;
    handle_t m_tx_itf;
  };

  /**
   * A cmd class that deletes a bridge_domain
   */
  class delete_cmd : public rpc_cmd<HW::item<bool>, rc_t, vapi::L2fib_add_del>
  {
  public:
    /**
     * Constructor
     */
    delete_cmd(HW::item<bool>& item, const mac_address_t& mac, uint32_t id);

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);

    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const;

    /**
     * Comparison operator - only used for UT
     */
    bool operator==(const delete_cmd& i) const;

  private:
    mac_address_t m_mac;
    uint32_t m_bd;
  };

  /**
   * A cmd class that Dumps all the interface spans
   */
  class dump_cmd : public VOM::dump_cmd<vapi::L2_fib_table_dump>
  {
  public:
    /**
     * Constructor
     */
    dump_cmd();
    dump_cmd(const dump_cmd& d);

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);
    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const;

    /**
     * Comparison operator - only used for UT
     */
    bool operator==(const dump_cmd& i) const;

  private:
    /**
     * HW reutrn code
     */
    HW::item<bool> item;
  };

private:
  /**
   * Class definition for listeners to OM events
   */
  class event_handler : public OM::listener, public inspect::command_handler
  {
  public:
    event_handler();
    virtual ~event_handler() = default;

    /**
     * Handle a populate event
     */
    void handle_populate(const client_db::key_t& key);

    /**
     * Handle a replay event
     */
    void handle_replay();

    /**
     * Show the object in the Singular DB
     */
    void show(std::ostream& os);

    /**
     * Get the sortable Id of the listener
     */
    dependency_t order() const;
  };

  /**
   * event_handler to register with OM
   */
  static event_handler m_evh;

  /**
   * Commit the acculmulated changes into VPP. i.e. to a 'HW" write.
   */
  void update(const bridge_domain_entry& obj);

  /**
   * Find or add the instnace of the bridge_domain domain in the OM
   */
  static std::shared_ptr<bridge_domain_entry> find_or_add(
    const bridge_domain_entry& temp);

  /*
   * It's the VPPHW class that updates the objects in HW
   */
  friend class OM;

  /**
   * It's the singular_db class that calls replay()
   */
  friend class singular_db<key_t, bridge_domain_entry>;

  /**
   * Sweep/reap the object if still stale
   */
  void sweep(void);

  /**
   * HW configuration for the result of creating the bridge_domain
   */
  HW::item<bool> m_hw;

  /**
   * The mac to match
   */
  mac_address_t m_mac;

  /**
   * The bridge_domain domain the bridge_domain is in.
   */
  std::shared_ptr<bridge_domain> m_bd;

  /**
   * The set of paths
   */
  std::shared_ptr<interface> m_tx_itf;

  /**
   * A map of all bridge_domains
   */
  static singular_db<key_t, bridge_domain_entry> m_db;
};

std::ostream& operator<<(std::ostream& os,
                         const bridge_domain_entry::key_t& key);
};

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "mozilla")
 * End:
 */

#endif
