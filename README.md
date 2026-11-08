# Order Cache Project

Implementation of an in-memory cache for order objects, facilitating operations like adding, removing, and matching buy and sell orders. This simulates a basic trading platform.

- An "order" is a request to buy or sell a financial security (eg. bond, stock, commodity, etc.).
- Each order is uniquely identified by an order id.
- Each security has a different security id.
- Order matching occurs for orders with the same security id, different side (buy or sell), and different company (company of person who requested the order).


# Performance notes

1 million orders are matched in 5-10 ms (measured in the last 2 tests in OrderCacheTest.cpp), getMatchingSizeForSecurity(secId) uses linear algorithm.
Single threaded execution purely on CPU (AMD Ryzen 9HX)