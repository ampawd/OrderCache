#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class Order
{
public:

    Order(
        const std::string& ordId,
        const std::string& secId,
        const std::string& side,
        const unsigned int qty,
        const std::string& user,
        const std::string& company)
        : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) {}

    std::string orderId() const    { return m_orderId; }
    std::string securityId() const { return m_securityId; }
    std::string side() const       { return m_side; }
    std::string user() const       { return m_user; }
    std::string company() const    { return m_company; }
    unsigned int qty() const       { return m_qty; }

private:

    // use the below to hold the order data
    // do not remove the these member variables
    std::string m_orderId;     // unique order id
    std::string m_securityId;  // security identifier
    std::string m_side;        // side of the order, eg Buy or Sell
    unsigned int m_qty;        // qty for this order
    std::string m_user;        // user name who owns this order
    std::string m_company;     // company for user
};

class OrderCacheInterface
{
public:
    // add order to the cache
    virtual void addOrder(Order order) = 0;

    // remove order with this unique order id from the cache
    virtual void cancelOrder(const std::string& orderId) = 0;

    // remove all orders in the cache for this user
    virtual void cancelOrdersForUser(const std::string& user) = 0;

    // remove all orders in the cache for this security with qty >= minQty
    virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) = 0;

    // return the total qty that can match for the security id
    virtual unsigned int getMatchingSizeForSecurity(const std::string& securityId) = 0;

    // return all orders in cache in a vector
    virtual std::vector<Order> getAllOrders() const = 0;
};

class OrderCache : public OrderCacheInterface
{
public:
    void addOrder(Order order) override;  

    void cancelOrder(const std::string& orderId) override;

    void cancelOrdersForUser(const std::string& user) override;

    void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override;

    unsigned int getMatchingSizeForSecurity(const std::string& securityId) override;

    std::vector<Order> getAllOrders() const override;

    void setNumOrdersInCache(unsigned int n) 
    {
        m_orders.reserve(n);
        m_sell_orders.reserve(n);
        m_buy_orders.reserve(n);
        m_userIndex.reserve(n);
    }
    
    bool validateOrder(const Order& order) const;      

private:
    std::unordered_map<std::string, Order> m_orders;   // orderId -> Order

    std::unordered_map<std::string, std::vector<Order>> m_sell_orders; // secId -> Sell Order

    std::unordered_map<std::string, std::vector<Order>> m_buy_orders;  // secId -> Buy Order

    std::unordered_map<std::string, std::unordered_set<std::string>> m_userIndex;  // userName -> orderIds
};
