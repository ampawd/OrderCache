#include "OrderCache.h"

#include <iostream>

bool OrderCache::validateOrder(const Order& order) const
{
	if (order.qty() == 0) { return false; }

	auto side = order.side();
	for (char& c: side) { c = std::tolower(c); }
	if (side != "buy" && side != "sell") { return false; }

	if (order.securityId().empty() || order.company().empty() || order.user().empty()) { return false; }

	return true;
}

void OrderCache::addOrder(Order order)
{
	if (!validateOrder(order))
	{
		throw std::runtime_error(R"(order validation failed, "please ensure that: 
			qty is > 0, 
			side is either Buy or Sell, 
			security, company and user are not empty)");
	}

	if (order.side() == "Sell")
	{
		m_sell_orders[order.securityId()].reserve(1000000u);
		m_sell_orders[order.securityId()].push_back(order);		
	}
	else
	{
		m_buy_orders[order.securityId()].reserve(1000000u);
		m_buy_orders[order.securityId()].push_back(order);
	}

	if (m_orders.find(order.orderId()) != m_orders.end())
	{
		throw std::runtime_error("Can not have orders with same orderId.");
	}

	m_userIndex[order.user()].insert(order.orderId());
	m_orders.insert({ order.orderId(), std::move(order) });
}

void OrderCache::cancelOrder(const std::string& orderId)
{
	if (orderId.empty()) return;

	auto it = m_orders.find(orderId);
	if (it == m_orders.end()) return;

	const Order& order = it->second;
	const auto& secId = order.securityId();
	
	if (order.side() == "Buy")
	{
		if (auto it = m_buy_orders.find(secId); it != m_buy_orders.end())
		{
			auto& book = it->second;
			book.erase(std::remove_if(book.begin(), book.end(),
				[&orderId](const auto& order) { return order.orderId() == orderId; }), book.end());
		}
	}
	else
	{
		if (auto it = m_sell_orders.find(secId); it != m_sell_orders.end())
		{
			auto& book = it->second;
			book.erase(std::remove_if(book.begin(), book.end(),
				[&orderId](const auto& order) { return order.orderId() == orderId; }), book.end());
		}
	}

	// Clean up user index
	auto userIt = m_userIndex.find(order.user());
	if (userIt != m_userIndex.end()) 
	{
		userIt->second.erase(orderId);
		if (userIt->second.empty()) 
		{
			m_userIndex.erase(userIt);
		}
	}

	// Remove from primary map
	m_orders.erase(it);
}


void OrderCache::cancelOrdersForUser(const std::string& user) 
{
	if (user.empty()) return;
	auto it = m_userIndex.find(user);
	if (it == m_userIndex.end()) return;

	std::unordered_set<std::string> affectedSecurities;
	auto orderIdsForUser = it->second;
	for (const auto& orderId : orderIdsForUser)
	{
		auto it = m_orders.find(orderId);
		if (it == m_orders.end()) continue;

		affectedSecurities.insert(it->second.securityId());
		m_orders.erase(it);
	}

	for (const auto& secId : affectedSecurities)
	{
		if (auto buyIt = m_buy_orders.find(secId); buyIt != m_buy_orders.end())
		{
			auto& book = buyIt->second;
			book.erase(std::remove_if(book.begin(), book.end(),
				[&orderIdsForUser](const Order& o) {
					return orderIdsForUser.count(o.orderId()) > 0;
				}), book.end());
		}
		if (auto sellIt = m_sell_orders.find(secId); sellIt != m_sell_orders.end())
		{
			auto& book = sellIt->second;
			book.erase(std::remove_if(book.begin(), book.end(),
				[&orderIdsForUser](const Order& o) {
					return orderIdsForUser.count(o.orderId()) > 0;
				}), book.end());
		}
	}

	m_userIndex.erase(user);
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) 
{
	if (securityId.empty()) return;

	auto buyIt = m_buy_orders.find(securityId);
	auto sellIt = m_sell_orders.find(securityId);
	std::vector<Order> canceledOrders;
	size_t reserveSize = 0;
	bool buyOrdersExists = buyIt != m_buy_orders.end();
	bool sellOrdersExists = sellIt != m_sell_orders.end();
	if (!buyOrdersExists && !sellOrdersExists) return;

	if (buyOrdersExists) reserveSize += buyIt->second.size();
	if (sellOrdersExists) reserveSize += sellIt->second.size();
	
	canceledOrders.reserve(reserveSize);
	auto f = [minQty, &canceledOrders](const auto& order) {
		if (order.qty() >= minQty)
		{
			canceledOrders.push_back(order);
			return true;
		}
		return false;
	};

	if (buyOrdersExists)
	{
		auto& buyBook = buyIt->second;
		buyBook.erase(std::remove_if(buyBook.begin(), buyBook.end(), f), buyBook.end());
	}
	if (sellOrdersExists)
	{
		auto& sellBook = sellIt->second;
		sellBook.erase(std::remove_if(sellBook.begin(), sellBook.end(), f), sellBook.end());
	}

	for (const auto& order : canceledOrders)
	{
		const std::string& orderId = order.orderId();		
		auto userIt = m_userIndex.find(order.user());
		if (userIt != m_userIndex.end())
		{
			userIt->second.erase(orderId);
			if (userIt->second.empty())
			{
				m_userIndex.erase(userIt);
			}
		}
		if (auto ordIt = m_orders.find(orderId); ordIt != m_orders.end())
		{
			m_orders.erase(ordIt);
		}
	}
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) 
{
	if (securityId.empty()) return 0;

	const auto& it_sell = m_sell_orders.find(securityId);
	const auto& it_buy = m_buy_orders.find(securityId);

	if (it_sell == m_sell_orders.end() || it_buy == m_buy_orders.end()) return 0;

	const auto& sec_id_sell_orders = it_sell->second;
	const auto& sec_id_buy_orders = it_buy->second;

	struct CompanyQty
	{
		unsigned sell_qty = 0;
		unsigned buy_qty = 0;
		std::string name;
	};

	std::vector<CompanyQty> unique_companies;
	unique_companies.reserve(sec_id_sell_orders.size() + sec_id_buy_orders.size());

	// sum sell and buy qtys per company
	auto addQty = [&unique_companies](const std::string& name, unsigned int qty, bool is_sell) {

		auto it = std::find_if(unique_companies.begin(), unique_companies.end(),
			[&](const CompanyQty& c) { return c.name == name; });

		if (it != unique_companies.end()) 
		{
			if (is_sell) 
				it->sell_qty += qty;
			else 
				it->buy_qty += qty;
		}
		else 
		{
			CompanyQty c{0, 0, name};
			if (is_sell) 
				c.sell_qty = qty;
			else 
				c.buy_qty = qty;

			unique_companies.push_back(c);
		}
	};

	unsigned int total_sell = 0;
	unsigned int total_buy = 0;

	for (const auto& order : sec_id_sell_orders)
	{
		addQty(order.company(), order.qty(), true);
		total_sell += order.qty();
	}

	for (const auto& order : sec_id_buy_orders)
	{
		addQty(order.company(), order.qty(), false);
		total_buy += order.qty();
	}

	unsigned int max_possible_match = std::min(total_sell, total_buy);

	// Check if any single company restricts the match due to the self-trading rule
	for (const auto& company : unique_companies)
	{
		unsigned int s_i = company.sell_qty; 
		unsigned int b_i = company.buy_qty;  

		// Maximum units this specific company can clear with everyone else
		unsigned int max_company_clearance = (total_sell - s_i) + (total_buy - b_i);

		if (max_possible_match > max_company_clearance)
		{
			max_possible_match = max_company_clearance;
		}
	}

	return max_possible_match;
}

std::vector<Order> OrderCache::getAllOrders() const
{
	std::vector<Order> result;
	result.reserve(m_orders.size());		
	
	for (const auto& [_, order] : m_orders) 
	{
		result.push_back(order);
	}

	return result;
}