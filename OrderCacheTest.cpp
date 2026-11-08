#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include "OrderCache.h"
#include "gtest/gtest.h"

class OrderCacheTest : public ::testing::Test 
{
protected:
    OrderCache cache;

    void SetUp() override
    {
        cache.setNumOrdersInCache(1000);
    }
};

TEST_F(OrderCacheTest, Example1GetMatchingSizeForSec)
{
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB" });
    cache.addOrder(Order{ "OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA" });
    cache.addOrder(Order{ "OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC" });
    cache.addOrder(Order{ "OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB" });
    cache.addOrder(Order{ "OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD" });
    cache.addOrder(Order{ "OrdId7", "SecId2", "Buy", 2000,  "User7", "CompanyE" });
    cache.addOrder(Order{ "OrdId8", "SecId2", "Sell", 5000,  "User8", "CompanyE" });
    
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 0);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 2700);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId3"), 0);
}

TEST_F(OrderCacheTest, Example2GetMatchingSizeForSec)
{
    cache.addOrder(Order{ "OrdId1", "SecId1", "Sell", 100, "User10", "Company2" });
    cache.addOrder(Order{ "OrdId2", "SecId3", "Sell", 200, "User8", "Company2" });
    cache.addOrder(Order{ "OrdId3", "SecId1", "Buy", 300, "User13", "Company2" });
    cache.addOrder(Order{ "OrdId4", "SecId2", "Sell", 400, "User12", "Company2" });
    cache.addOrder(Order{ "OrdId5", "SecId3", "Sell", 500, "User7", "Company2" });
    cache.addOrder(Order{ "OrdId6", "SecId3", "Buy", 600, "User3", "Company1" });
    cache.addOrder(Order{ "OrdId7", "SecId1", "Sell", 700, "User10", "Company2" });
    cache.addOrder(Order{ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    cache.addOrder(Order{ "OrdId9", "SecId2", "Buy", 900, "User6", "Company2" });
    cache.addOrder(Order{ "OrdId10", "SecId2", "Sell", 1000, "User5", "Company1" });
    cache.addOrder(Order{ "OrdId11", "SecId1", "Sell", 1100, "User13", "Company2" });
    cache.addOrder(Order{ "OrdId12", "SecId2", "Buy", 1200, "User9", "Company2" });
    cache.addOrder(Order{ "OrdId13", "SecId1", "Sell", 1300, "User1", "Company1" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 300);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 1000);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId3"), 600);
}

TEST_F(OrderCacheTest, Example3GetMatchingSizeForSec)
{
    cache.addOrder(Order{ "OrdId1", "SecId3", "Sell", 100, "User1", "Company1" });
    cache.addOrder(Order{ "OrdId2", "SecId3", "Sell", 200, "User3", "Company2" });
    cache.addOrder(Order{ "OrdId3", "SecId1", "Buy", 300, "User2", "Company1" });
    cache.addOrder(Order{ "OrdId4", "SecId3", "Sell", 400, "User5", "Company2" });
    cache.addOrder(Order{ "OrdId5", "SecId2", "Sell", 500, "User2", "Company1" });
    cache.addOrder(Order{ "OrdId6", "SecId2", "Buy", 600, "User3", "Company2" });
    cache.addOrder(Order{ "OrdId7", "SecId2", "Sell", 700, "User1", "Company1" });
    cache.addOrder(Order{ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    cache.addOrder(Order{ "OrdId9", "SecId1", "Buy", 900, "User5", "Company2" });
    cache.addOrder(Order{ "OrdId10", "SecId1", "Sell", 1000, "User1", "Company1" });
    cache.addOrder(Order{ "OrdId11", "SecId2", "Sell", 1100, "User6", "Company2" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 900);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 600);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId3"), 0);
}


// Standard matching logic across different companies with partial fills
TEST_F(OrderCacheTest, PartialAndMultipleMatches)
{
    // SecId1 Setup
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId1", "Sell", 400, "User2", "CompanyB" });
    cache.addOrder(Order{ "OrdId3", "SecId1", "Sell", 800, "User3", "CompanyC" }); // Total Sell 1200, Buy 1000

    // SecId2 Setup - Exact match across different companies
    cache.addOrder(Order{ "OrdId4", "SecId2", "Buy", 500, "User4", "CompanyA" });
    cache.addOrder(Order{ "OrdId5", "SecId2", "Sell", 500, "User5", "CompanyB" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 1000);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 500);
}

// Standard compliance rules - Orders from the same company must NOT match
TEST_F(OrderCacheTest, PreventSameCompanyMatching)
{
    // Same company, opposite sides -> Should not match
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId1", "Sell", 1000, "User2", "CompanyA" });

    // Mix of same company and different company
    cache.addOrder(Order{ "OrdId3", "SecId2", "Buy", 1000, "User3", "CompanyA" });
    cache.addOrder(Order{ "OrdId4", "SecId2", "Sell", 600, "User4", "CompanyA" }); // No match (Same Company)
    cache.addOrder(Order{ "OrdId5", "SecId2", "Sell", 500, "User5", "CompanyB" }); // Matches 500 with OrdId3

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 0);
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 500);
}

TEST_F(OrderCacheTest, MatchingSizeAfterOrderRemoval)
{
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId1", "Sell", 1000, "User2", "CompanyB" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 1000);

    cache.cancelOrder("OrdId1");

    // Matching size should now drop to 0
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 0);

    ASSERT_EQ(cache.getAllOrders().size(), 1);
}

TEST_F(OrderCacheTest, MatchingSizeAfterOrdersRemovalForUser)
{
    cache.addOrder(Order{ "OrdId1", "SecId4", "Sell", 1000, "User4", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId1", "Sell", 1000, "User3", "CompanyD" });
    cache.addOrder(Order{ "OrdId3", "SecId2", "Buy", 500, "User3", "CompanyC" });
    cache.addOrder(Order{ "OrdId4", "SecId4", "Buy", 400, "User1", "CompanyB" });
    cache.addOrder(Order{ "OrdId5", "SecId1", "Sell", 1200, "User3", "CompanyB" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 0);

    cache.cancelOrdersForUser("User3");

    ASSERT_EQ(cache.getAllOrders().size(), 2);

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId4"), 400);

    cache.cancelOrdersForUser("User1");
    ASSERT_EQ(cache.getAllOrders().size(), 1);

    // no buy orders for SecId4, thus match is 0
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId4"), 0);
}

TEST_F(OrderCacheTest, MatchingSizeAfterOrdersRemovalForSecIdMinQty)
{
    cache.addOrder(Order{ "OrdId1", "SecId4", "Sell", 1000, "User2", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId4", "Buy", 500, "User4", "CompanyA" });
    cache.addOrder(Order{ "OrdId3", "SecId1", "Sell", 1000, "User3", "CompanyD" });
    cache.addOrder(Order{ "OrdId4", "SecId2", "Buy", 500, "User3", "CompanyC" });
    cache.addOrder(Order{ "OrdId5", "SecId2", "Buy", 300, "User3", "CompanyC" });
    cache.addOrder(Order{ "OrdId6", "SecId4", "Buy", 400, "User1", "CompanyB" });
    cache.addOrder(Order{ "OrdId7", "SecId2", "Sell", 1200, "User3", "CompanyB" });
    cache.addOrder(Order{ "OrdId8", "SecId2", "Sell", 200, "User3", "CompanyA" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 800);

    // all orders with qty >= 500 will be canceled (i.e. removed)
    cache.cancelOrdersForSecIdWithMinimumQty("SecId2", 500);

    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId2"), 200);

    ASSERT_EQ(cache.getAllOrders().size(), 6);
}

// Empty cache and missing security lookups
TEST_F(OrderCacheTest, EmptyCacheAndNonExistentSecurity)
{
    // Cache is empty
    ASSERT_EQ(cache.getMatchingSizeForSecurity("SecId1"), 0);

    // Add orders for one security, query a completely different ones
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    cache.addOrder(Order{ "OrdId2", "SecId1", "Sell", 1000, "User2", "CompanyB" });

    ASSERT_EQ(cache.getMatchingSizeForSecurity("NonExistentSec"), 0);
    ASSERT_EQ(cache.getMatchingSizeForSecurity(""), 0);
}

// verify that invalid order causes exceptions to be thrown
TEST_F(OrderCacheTest, InvalidOrder)
{
    cache.addOrder(Order{ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });    

    try
    {
        cache.addOrder(Order{ "OrdId1", "SecId1", "Sell", 1000, "User2", "CompanyB" });
        FAIL() << "Expected std::runtime_error was not thrown";
    }
    catch (const std::runtime_error& ex)
    {
        EXPECT_STREQ("Can not have orders with same orderId.", ex.what());
    }
    catch (...)
    {
        FAIL() << "Expected std::runtime_error, but caught a different exception type";
    }

    EXPECT_THROW(cache.addOrder(Order{ "OrdId3", "SecId1", "Sell", std::numeric_limits<unsigned int>::max() + 1, "User2", "CompanyB"}), std::runtime_error);
    EXPECT_THROW(cache.addOrder(Order{ "OrdId4", "SecId1", "Sell", 0, "User2", "CompanyB" }), std::runtime_error);
    EXPECT_THROW(cache.addOrder(Order{ "OrdId5", "", "Sell", 0, "User2", "CompanyB" }), std::runtime_error);
    EXPECT_THROW(cache.addOrder(Order{ "OrdId5", "SecId2", "Sell", 0, "User2", "" }), std::runtime_error);
    EXPECT_THROW(cache.addOrder(Order{ "OrdId5", "SecId2", "InvalidSide", 0, "User2", "CompanyC" }), std::runtime_error);
}


//////////////////////////////////////////////////////// testing OrderCache for large number of orders \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

class OrderCacheTestForLargeInput : public ::testing::Test
{
protected:
    OrderCache cache;
    const std::string targetSecId = "SEC_LARGE_TEST";

    void SetUp() override
    {
        cache.setNumOrdersInCache(2000000u);
    }
};

TEST_F(OrderCacheTestForLargeInput, GetMatchingSizeForSecurityTest)
{
    std::chrono::time_point<std::chrono::steady_clock> start, end;

    const unsigned int buyCountA = 300000;
    const unsigned int sellCountA = 100000;
    const unsigned int buyCountB = 200000;
    const unsigned int sellCountB = 400000;

    for (unsigned int i = 0; i < buyCountA; ++i)
    {
        cache.addOrder(Order("B_A_" + std::to_string(i), targetSecId, "Buy", 10, "UserA", "CompanyA"));
    }
    for (unsigned int i = 0; i < sellCountA; ++i)
    {
        cache.addOrder(Order("S_A_" + std::to_string(i), targetSecId, "Sell", 5, "UserA", "CompanyA"));
    }

    for (unsigned int i = 0; i < buyCountB; ++i)
    {
        cache.addOrder(Order("B_B_" + std::to_string(i), targetSecId, "Buy", 5, "UserB", "CompanyB"));
    }
    for (unsigned int i = 0; i < sellCountB; ++i)
    {
        cache.addOrder(Order("S_B_" + std::to_string(i), targetSecId, "Sell", 10, "UserB", "CompanyB"));
    }

    std::cout << "Orders have been added, running the matching.\n";
    start = std::chrono::steady_clock::now();
    unsigned int actualMatch = cache.getMatchingSizeForSecurity(targetSecId);
    end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "getMatchingSizeForSecurity took: "<< duration.count() << " ms.\n";

    unsigned int expectedMatch = 3500000u;
    ASSERT_EQ(actualMatch, expectedMatch);
}

TEST_F(OrderCacheTestForLargeInput, GetMatchingSizeForSecurityTestWithCancelations)
{
    std::chrono::time_point<std::chrono::steady_clock> start, end;

    const unsigned int buyCountA = 200000;
    const unsigned int sellCountB = 300000;

    for (unsigned int i = 0; i < buyCountA; ++i)
    {
        cache.addOrder(Order("B_A_" + std::to_string(i), targetSecId, "Buy", 10, "UserA", "CompanyA"));
    }
    for (unsigned int i = 0; i < sellCountB; ++i)
    {
        cache.addOrder(Order("S_B_" + std::to_string(i), targetSecId, "Sell", 10, "UserB", "CompanyB"));
    }

    std::cout << "Orders have been added, running the matching.\n";
    start = std::chrono::steady_clock::now();
    unsigned int actualMatch = cache.getMatchingSizeForSecurity(targetSecId);
    end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "getMatchingSizeForSecurity took: " << duration.count() << " ms.\n";

    unsigned int expectedMatch = 2000000u;
    ASSERT_EQ(actualMatch, expectedMatch);

    for (unsigned int i = 0; i < 25; ++i)
    {
        cache.cancelOrder("S_B_" + std::to_string(i));
    }

    start = std::chrono::steady_clock::now();
    actualMatch = cache.getMatchingSizeForSecurity(targetSecId);
    end = std::chrono::steady_clock::now();

    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "getMatchingSizeForSecurity took: " << duration.count() << " ms.\n";


    ASSERT_EQ(actualMatch, 2000000u);
}


int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
