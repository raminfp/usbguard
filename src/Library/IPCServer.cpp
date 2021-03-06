//
// Copyright (C) 2015 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>
//
#include "IPCServerPrivate.hpp"
#include "Common/Utility.hpp"

namespace usbguard
{
  static const std::vector<std::pair<String,IPCServer::AccessControl::Section>> section_ttable = {
    { "ALL", IPCServer::AccessControl::Section::ALL },
    { "Policy", IPCServer::AccessControl::Section::POLICY },
    { "Parameters", IPCServer::AccessControl::Section::PARAMETERS },
    { "Devices", IPCServer::AccessControl::Section::DEVICES },
    { "Exceptions", IPCServer::AccessControl::Section::EXCEPTIONS },
    { "None", IPCServer::AccessControl::Section::NONE }
  };

  IPCServer::AccessControl::Section IPCServer::AccessControl::sectionFromString(const std::string& section_string)
  {
    for (auto ttable_entry : section_ttable) {
      if (ttable_entry.first == section_string) {
        return ttable_entry.second;
      }
    }
    throw std::runtime_error("Invalid AccessControl::Section string");
  }

  static const std::vector<std::pair<String,IPCServer::AccessControl::Privilege>> privilege_ttable = {
    { "ALL", IPCServer::AccessControl::Privilege::ALL },
    { "modify", IPCServer::AccessControl::Privilege::MODIFY },
    { "list", IPCServer::AccessControl::Privilege::LIST },
    { "listen", IPCServer::AccessControl::Privilege::LISTEN },
    { "none", IPCServer::AccessControl::Privilege::NONE }
  };

  IPCServer::AccessControl::Privilege IPCServer::AccessControl::privilegeFromString(const std::string& privilege_string)
  {
    for (auto ttable_entry : privilege_ttable) {
      if (ttable_entry.first == privilege_string) {
        return ttable_entry.second;
      }
    }
    throw std::runtime_error("Invalid AccessControl::Section string");
  }

  IPCServer::AccessControl::AccessControl()
  {
    /* Empty: no privileges */
  }

  IPCServer::AccessControl::AccessControl(IPCServer::AccessControl::Section section, IPCServer::AccessControl::Privilege privilege)
  {
    setPrivilege(section, privilege);
  }

  IPCServer::AccessControl::AccessControl(const IPCServer::AccessControl& rhs)
    : _access_control(rhs._access_control)
  {
  }

  IPCServer::AccessControl& IPCServer::AccessControl::operator=(const IPCServer::AccessControl& rhs)
  {
    _access_control = rhs._access_control;
    return *this;
  }

  bool IPCServer::AccessControl::hasPrivilege(IPCServer::AccessControl::Section section, IPCServer::AccessControl::Privilege privilege) const
  {
    if (section == Section::ALL || section == Section::NONE) {
      throw USBGUARD_BUG("Cannot test against ALL, NONE sections");
    }

    const auto it = _access_control.find(section);

    if (it == _access_control.cend()) {
      return false;
    }

    return (it->second & static_cast<uint8_t>(privilege)) == static_cast<uint8_t>(privilege);
  }

  void IPCServer::AccessControl::setPrivilege(IPCServer::AccessControl::Section section, IPCServer::AccessControl::Privilege privilege)
  {
    if (section == Section::NONE) {
      throw USBGUARD_BUG("Cannot set privileges for NONE section");
    }
    if (section == Section::ALL) {
      for (const auto& value : {
            Section::POLICY,
            Section::PARAMETERS,
            Section::EXCEPTIONS,
            Section::DEVICES }) {
        _access_control[value] |= static_cast<uint8_t>(privilege);
      }
    }
    else {
      _access_control[section] |= static_cast<uint8_t>(privilege);
    }
  }

  void IPCServer::AccessControl::clear()
  {
    _access_control.clear();
  }

  void IPCServer::AccessControl::load(std::istream& stream)
  {
    std::string line;
    size_t line_number = 0;

    while (std::getline(stream, line)) {
      ++line_number;
      const size_t nv_separator = line.find_first_of("=");

      if (nv_separator == String::npos) {
        continue;
      }

      const String section_string = trim(line.substr(0, nv_separator));
      const Section section = sectionFromString(section_string);

      const String privileges_string = line.substr(nv_separator + 1);
      StringVector privilege_strings;
      tokenizeString(privileges_string, privilege_strings, " ", /*trim_empty=*/true);

      for (const String& privilege_string : privilege_strings) {
        const Privilege privilege = privilegeFromString(privilege_string);
        setPrivilege(section, privilege);
      }
    }
  }

  void IPCServer::AccessControl::merge(const IPCServer::AccessControl& rhs)
  {
    for (auto const& ac_entry : rhs._access_control) {
      _access_control[ac_entry.first] |= ac_entry.second;
    }
  }

  IPCServer::IPCServer()
  {
    d_pointer = new IPCServerPrivate(*this);
  }

  IPCServer::~IPCServer()
  {
    delete d_pointer;
  }

  void IPCServer::start()
  {
    d_pointer->start();
  }

  void IPCServer::stop()
  {
    d_pointer->stop();
  }

  void IPCServer::DevicePresenceChanged(uint32_t id,
                                        DeviceManager::EventType event,
                                        Rule::Target target,
                                        const std::string& device_rule)
  {
    d_pointer->DevicePresenceChanged(id, event, target, device_rule);
  }

  void IPCServer::DevicePolicyChanged(uint32_t id,
                                      Rule::Target target_old,
                                      Rule::Target target_new,
                                      const std::string& device_rule,
                                      uint32_t rule_id)
  {
    d_pointer->DevicePolicyChanged(id, target_old, target_new, device_rule, rule_id);
  }

  void IPCServer::ExceptionMessage(const std::string& context,
                                   const std::string& object,
                                   const std::string& reason)
  {
    d_pointer->ExceptionMessage(context, object, reason);
  }

  void IPCServer::addAllowedUID(uid_t uid, const IPCServer::AccessControl& ac)
  {
    d_pointer->addAllowedUID(uid, ac);
  }

  void IPCServer::addAllowedGID(gid_t gid, const IPCServer::AccessControl& ac)
  {
    d_pointer->addAllowedGID(gid, ac);
  }

  void IPCServer::addAllowedUsername(const std::string& username, const IPCServer::AccessControl& ac)
  {
    d_pointer->addAllowedUsername(username, ac);
  }

  void IPCServer::addAllowedGroupname(const std::string& groupname, const IPCServer::AccessControl& ac)
  {
    d_pointer->addAllowedGroupname(groupname, ac);
  }
} /* namespace usbguard */
