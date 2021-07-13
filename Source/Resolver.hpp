/**
 * @file
 * @brief Resolver class.
 */

#pragma once

#include <string>
#include <memory>
#include <functional>

#include "KString.hpp"
#include "Address.hpp"
#include "Port.hpp"

struct ResolvedAddress
{
	std::string hostname;
	Country country;
	ASN asn;

	ResolvedAddress() = default;
};

struct ResolvedPort
{
	std::string service;

	ResolvedPort() = default;
};

/**
 * @brief Resolver for obtaining information about addresses and ports.
 */
class Resolver
{
public:
	using CallbackHostname = std::function<void(std::string&, AddressPack&, void*)>;
	using CallbackService = std::function<void(std::string&, EPortType, PortPack&, void*)>;
	using CallbackAddress = std::function<void(AddressData&, ResolvedAddress&, void*)>;
	using CallbackPort = std::function<void(PortData&, ResolvedPort&, void*)>;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;

	static AddressPack PlatformResolveHostname(const KString & hostname);
	static PortPack PlatformResolveService(const KString & service, EPortType portType);
	static std::string PlatformResolveAddress(const IAddress & address);
	static std::string PlatformResolvePort(const Port & port);

public:
	Resolver();
	~Resolver();

	bool isAddressHostnameEnabled() const;
	bool isPortServiceEnabled() const;

	void resolveHostname(std::string hostname, const CallbackHostname & callback, void *param);
	void resolveService(std::string service, EPortType type, const CallbackService & callback, void *param);
	void resolveAddress(AddressData & address, const CallbackAddress & callback, void *param);
	void resolvePort(PortData & port, const CallbackPort & callback, void *param);
};
