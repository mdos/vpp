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

#ifndef __VOM_TAP_INTERFACE_H__
#define __VOM_TAP_INTERFACE_H__

#include "vom/interface.hpp"

namespace VOM {
/**
 * A tap-interface. e.g. a tap interface
 */
class tap_interface : public interface
{
public:
  tap_interface(const std::string& name,
                admin_state_t state,
                route::prefix_t prefix);

  tap_interface(const std::string& name,
                admin_state_t state,
                route::prefix_t prefix,
                const l2_address_t& l2_address);

  ~tap_interface();
  tap_interface(const tap_interface& o);

  /**
   * Return the matching 'singular instance' of the TAP interface
   */
  std::shared_ptr<tap_interface> singular() const;

  /**
   * A functor class that creates an interface
   */
  class create_cmd : public interface::create_cmd<vapi::Tap_connect>
  {
  public:
    create_cmd(HW::item<handle_t>& item,
               const std::string& name,
               route::prefix_t& prefix,
               const l2_address_t& l2_address);

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);
    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const;

  private:
    route::prefix_t& m_prefix;
    const l2_address_t& m_l2_address;
  };

  /**
   *
   */
  class delete_cmd : public interface::delete_cmd<vapi::Tap_delete>
  {
  public:
    delete_cmd(HW::item<handle_t>& item);

    /**
     * Issue the command to VPP/HW
     */
    rc_t issue(connection& con);
    /**
     * convert to string format for debug purposes
     */
    std::string to_string() const;
  };

  /**
   * A cmd class that Dumps all the Vpp Interfaces
   */
  class dump_cmd : public VOM::dump_cmd<vapi::Sw_interface_tap_dump>
  {
  public:
    /**
     * Default Constructor
     */
    dump_cmd();

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
  static event_handler m_evh;

  /**
   * Construct with a handle
   */
  tap_interface(const handle_t& hdl,
                const std::string& name,
                admin_state_t state,
                route::prefix_t prefix);

  /**
   * Ip Prefix
   */
  route::prefix_t m_prefix;

  l2_address_t m_l2_address;

  /**
   * interface is a friend so it can construct with handles
   */
  friend class interface;

  /**
   * Return the matching 'instance' of the sub-interface
   *  over-ride from the base class
   */
  std::shared_ptr<interface> singular_i() const;

  /**
   * Virtual functions to construct an interface create commands.
   */
  virtual std::queue<cmd*>& mk_create_cmd(std::queue<cmd*>& cmds);

  /**
   * Virtual functions to construct an interface delete commands.
   */
  virtual std::queue<cmd*>& mk_delete_cmd(std::queue<cmd*>& cmds);

  /*
   * It's the OM class that call singular()
   */
  friend class OM;
};
}

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "mozilla")
 * End:
 */

#endif
