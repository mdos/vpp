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

#ifndef __VOM_NEIGHBOUR_H__
#define __VOM_NEIGHBOUR_H__

#include "vom/dump_cmd.hpp"
#include "vom/interface.hpp"
#include "vom/singular_db.hpp"
#include "vom/types.hpp"

#include <vapi/l2.api.vapi.hpp>

namespace VOM {
/**
 * A entry in the ARP termination table of a Bridge Domain
 */
class neighbour : public object_base
{
public:
  /**
   * The key for a bridge_domain ARP entry;
   *  the BD, IP address and MAC address
   */
  typedef std::tuple<interface::key_type,
                     mac_address_t,
                     boost::asio::ip::address>
    key_t;

  /**
   * Construct an ARP entry
   */
  neighbour(const interface& itf,
            const mac_address_t& mac,
            const boost::asio::ip::address& ip_addr);

  /**
   * Copy Construct
   */
  neighbour(const neighbour& r);

  /**
   * Destructor
   */
  ~neighbour();

  /**
   * Return the matching 'singular instance'
   */
  std::shared_ptr<neighbour> singular() const;

  /**
   * Find the instnace of the bridge_domain domain in the OM
   */
  static std::shared_ptr<neighbour> find(const neighbour& temp);

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
   * A command class that creates or updates the bridge domain ARP Entry
   */
  class create_cmd
    : public rpc_cmd<HW::item<bool>, rc_t, vapi::Ip_neighbor_add_del>
  {
  public:
    /**
     * Constructor
     */
    create_cmd(HW::item<bool>& item,
               handle_t itf,
               const mac_address_t& mac,
               const boost::asio::ip::address& ip_addr);

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
    handle_t m_itf;
    mac_address_t m_mac;
    boost::asio::ip::address m_ip_addr;
  };

  /**
   * A cmd class that deletes a bridge domain ARP entry
   */
  class delete_cmd
    : public rpc_cmd<HW::item<bool>, rc_t, vapi::Ip_neighbor_add_del>
  {
  public:
    /**
     * Constructor
     */
    delete_cmd(HW::item<bool>& item,
               handle_t itf,
               const mac_address_t& mac,
               const boost::asio::ip::address& ip_addr);

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
    handle_t m_itf;
    mac_address_t m_mac;
    boost::asio::ip::address m_ip_addr;
  };

  /**
   * A cmd class that Dumps all the neighbours
   */
  class dump_cmd : public VOM::dump_cmd<vapi::Ip_neighbor_dump>
  {
  public:
    /**
     * Constructor
     */
    dump_cmd(const handle_t& itf, const l3_proto_t& proto);
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

    /**
     * The interface to dump
     */
    handle_t m_itf;

    /**
     * V4 or V6
     */
    l3_proto_t m_proto;
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
  void update(const neighbour& obj);

  /**
   * Do the populate work
   */
  static void populate_i(const client_db::key_t& key,
                         std::shared_ptr<interface> itf,
                         const l3_proto_t& proto);

  /**
   * Find or add the instnace of the bridge_domain domain in the OM
   */
  static std::shared_ptr<neighbour> find_or_add(const neighbour& temp);

  /*
   * It's the VPPHW class that updates the objects in HW
   */
  friend class OM;

  /**
   * It's the singular_db class that calls replay()
   */
  friend class singular_db<key_t, neighbour>;

  /**
   * Sweep/reap the object if still stale
   */
  void sweep(void);

  /**
   * HW configuration for the result of creating the bridge_domain
   */
  HW::item<bool> m_hw;

  /**
   * The bridge_domain domain the bridge_domain is in.
   */
  std::shared_ptr<interface> m_itf;

  /**
   * The mac to match
   */
  mac_address_t m_mac;

  /**
   * The IP address
   */
  boost::asio::ip::address m_ip_addr;

  /**
   * A map of all bridge_domains
   */
  static singular_db<key_t, neighbour> m_db;
};

std::ostream& operator<<(std::ostream& os, const neighbour::key_t& key);
};

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "mozilla")
 * End:
 */

#endif
