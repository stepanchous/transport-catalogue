#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <unordered_map>

#include "../src/geo.h"
#include "../src/input_reader.h"
#include "../src/transport_catalogue.h"
#include "test_framework.h"

namespace trc {

namespace test {

using namespace std;

#if defined(INPUT_READER) || defined(ALL)
class InputReader : io::InputReader {
   public:
    void operator()() {
        RUN_TEST(TestMatchType);
        RUN_TEST(TestDefineType);
        RUN_TEST(TestParseStopRequest);
        RUN_TEST(TestExtractRoute);
        RUN_TEST(TestParseBusRequest);
        RUN_TEST(TestFormResult);
        RUN_TEST(TestFormResultEmptyBuses);
        RUN_TEST(TestFormResultEmptyStops);
    }

    static void TestMatchType() {
        const string request1 = "Bus";
        const string request2 = "Stop";

        ASSERT_EQUAL(RequestType::MatchType(request1),
                     RequestType::RequestSpecifier::BUS);
        ASSERT_EQUAL(RequestType::MatchType(request2),
                     RequestType::RequestSpecifier::STOP);
    }

    static void TestDefineType() {
        const string request1 = "Bus";
        const string request2 = "Bus a a a";
        const string request3 = "      Bus a a a";
        const string request4 = "Stop a a a";
        const string request5 = "\tStop a a a";

        ASSERT_EQUAL(InputReader::DefineType(request1),
                     RequestType::RequestSpecifier::BUS);
        ASSERT_EQUAL(InputReader::DefineType(request2),
                     RequestType::RequestSpecifier::BUS);
        ASSERT_EQUAL(InputReader::DefineType(request3),
                     RequestType::RequestSpecifier::BUS);
        ASSERT_EQUAL(InputReader::DefineType(request4),
                     RequestType::RequestSpecifier::STOP);
        ASSERT_EQUAL(InputReader::DefineType(request5),
                     RequestType::RequestSpecifier::STOP);
    }

    static void TestParseStopRequest() {
        const string request1 =
            "Stop stop1: 0.1, -89.0, 10m to stop1, 20m to stop 3";
        const string request2 = "Stop stop 2: .1, 4.5  ";

        const auto [stop1, coordinates1, stops_to_distance1] =
            ParseStopRequest(request1);
        const auto [stop2, coordinates2, stops_to_distance2] =
            ParseStopRequest(request2);

        const auto [stop1_expected, coordinates1_expected,
                    stops_to_distance1_expected] =
            tuple<string, geo::Coordinates, unordered_map<string, double>>{
                "stop1", {0.1, -89.0}, {{"stop1", 10}, {"stop 3", 20}}};
        const auto [stop2_expected, coordinates2_expected,
                    stops_to_distance2_expected] =
            tuple<string, geo::Coordinates, unordered_map<string, double>>{
                "stop 2", {.1, 4.5}, {}};

        ASSERT_EQUAL(stop1, stop1_expected);
        ASSERT_EQUAL(stop2, stop2_expected);

        ASSERT_EQUAL(coordinates1, coordinates1_expected);
        ASSERT_EQUAL(coordinates2, coordinates2_expected);

        ASSERT_EQUAL(stops_to_distance1, stops_to_distance1_expected);
        ASSERT_EQUAL(stops_to_distance2, stops_to_distance2_expected);
    }

    static void TestParseBusRequest() {
        const string request1 = "Bus bus1: A B - C";
        const string request2 = "Bus bus2: A > B > C";
        const string request3 = "Bus bus name: A - B";

        const tuple<string, vector<string>> expected1 = {"bus1",
                                                         {"A B", "C", "A B"}};
        const tuple<string, vector<string>> expected2 = {"bus2",
                                                         {"A", "B", "C"}};
        const tuple<string, vector<string>> expected3 = {"bus name",
                                                         {"A", "B", "A"}};

        ASSERT_EQUAL(ParseBusRequest(request1), expected1);
        ASSERT_EQUAL(ParseBusRequest(request2), expected2);
        ASSERT_EQUAL(ParseBusRequest(request3), expected3);
    }

    static void TestExtractRoute() {
        stringstream request1("A > B > C > A");
        stringstream request2("A - B");
        stringstream request3("A > B > C D > A\n");

        const vector<string> expected1 = {"A", "B", "C", "A"};
        const vector<string> expected2 = {"A", "B", "A"};
        const vector<string> expected3 = {"A", "B", "C D", "A"};

        ASSERT_EQUAL(ExtractRoute(request1, '>'), expected1);
        ASSERT_EQUAL(ExtractRoute(request2, '-'), expected2);
        ASSERT_EQUAL(ExtractRoute(request3, '>'), expected3);
    }

    static void TestFormResult() {
        InputReader ir;

        ir.Poll();

        auto stops = ir.FormStopsInfo();
        auto buses = ir.FormBuses();

        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            expected_stops = {{"A", {55.2, -40}, {{"B", 1}, {"C", 2}}},
                              {"B", {1.0, 30.45}, {{"C", 3}}},
                              {"C", {0, 0}, {}}};
        vector<tuple<string, vector<string>>> expected_buses = {
            {"a", {"A", "B", "A"}}, {"b", {"A", "C", "A"}}};

        ASSERT_EQUAL(stops, expected_stops);
        ASSERT_EQUAL(buses, expected_buses);
    }

    static void TestFormResultEmptyBuses() {
        InputReader ir;

        ir.Poll();

        auto stops = ir.FormStopsInfo();
        auto buses = ir.FormBuses();

        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            expected_stops = {{"A", {55.2, -40}, {}}};
        vector<tuple<string, vector<string>>> expected_buses = {};

        ASSERT_EQUAL(stops, expected_stops);
        ASSERT_EQUAL(buses, expected_buses);
    }

    static void TestFormResultEmptyStops() {
        InputReader ir;

        ir.Poll();

        auto stops = ir.FormStopsInfo();
        auto buses = ir.FormBuses();

        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            expected_stops = {};
        vector<tuple<string, vector<string>>> expected_buses = {{"a", {"A"}}};

        ASSERT_EQUAL(stops, expected_stops);
        ASSERT_EQUAL(buses, expected_buses);
    }
};
#endif

#if 1  // defined(TRANSPORT_CATALOGUE) || defined(ALL)
class TransportCatalogue {
   public:
    void operator()() {
        RUN_TEST(TestAddStop);
        RUN_TEST(TestAddBus);
        RUN_TEST(TestGetStopProperties);
        RUN_TEST(TestGetStopInfo);
        RUN_TEST(TestGetBusInfo);
        RUN_TEST(TestConstructor);
    }

   private:
    static void TestAddStop() {
        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            stops_data = {
                {"A", {0, 0}, {}}, {"B", {1, 1}, {}}, {"C", {2, 2}, {}}};
        vector<tuple<string, vector<string>>> buses_data = {
            {"a", {"A", "B", "A"}}, {"b", {"C", "B", "C"}}};

        trc::TransportCatalogue tc(stops_data, buses_data);

        tc.AddStop("D", {3, 3});

        auto [stop_name, coordinates] = tc.GetStopProperties("D").value();

        auto [stop_name_expected, coordinates_expected] =
            tuple<string, trc::geo::Coordinates>{"D", {3, 3}};

        ASSERT_EQUAL(stop_name, stop_name_expected);
        ASSERT_EQUAL(coordinates, coordinates_expected);
    }

    static void TestAddBus() {
        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            stops_data = {
                {"A", {0, 0}, {}}, {"B", {1, 1}, {}}, {"C", {2, 2}, {}}};
        vector<tuple<string, vector<string>>> buses_data = {
            {"a", {"A", "B", "A"}}, {"b", {"C", "B", "C"}}};

        trc::TransportCatalogue tc(stops_data, buses_data);

        tc.AddBus("c", {"C", "B", "C"});

        auto [bus_name, stop_count, unique_stop_count, route_distance,
              curvature] = tc.GetBusInfo("c").value();

        auto [bus_name_expected, stop_count_expected,
              unique_stop_count_expected, route_distance_expected] =
            tuple<string, size_t, size_t, double>{"c", 3, 2, 0.0};

        ASSERT_EQUAL(bus_name, bus_name_expected);
        ASSERT_EQUAL(stop_count, stop_count_expected);
        ASSERT_EQUAL(unique_stop_count, unique_stop_count_expected);
    }

    static void TestGetStopProperties() {
        trc::TransportCatalogue tc;

        tc.AddStop("A", {0, 0});

        auto [stop_name, coordinates] = tc.GetStopProperties("A").value();

        auto [stop_name_expected, coordinates_expected] =
            tuple<string, geo::Coordinates>{"A", {0, 0}};

        ASSERT_EQUAL(stop_name, stop_name_expected);
        ASSERT_EQUAL(coordinates, coordinates_expected);
        ASSERT(!tc.GetStopProperties("B").has_value());
    }

    static void TestGetStopInfo() {
        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            stops_data = {{"A", {0, 0}, {}},
                          {"B", {1, 1}, {}},
                          {"C", {2, 2}, {}},
                          {"D", {3, 3}, {}}};
        vector<tuple<string, vector<string>>> buses_data = {
            {"a", {"A", "B", "A"}}, {"b", {"C", "A", "C"}}};

        trc::TransportCatalogue tc(stops_data, buses_data);

        string stop_name1 = "A";
        string stop_name2 = "D";
        string stop_name3 = "E";

        auto [name1, buses1] = tc.GetStopInfo(stop_name1).value();
        auto [name2, buses2] = tc.GetStopInfo(stop_name2).value();

        auto [name1_expected, buses1_expected] =
            tuple<string, set<string>>{"A", {"a", "b"}};
        auto [name2_expected, buses2_expected] =
            tuple<string, set<string>>{"D", {}};

        ASSERT_EQUAL(name1, name1_expected);
        ASSERT_EQUAL(buses1, buses1_expected);
        ASSERT_EQUAL(name2, name2_expected);
        ASSERT_EQUAL(buses2, buses2_expected);
        ASSERT(!tc.GetStopInfo(stop_name3).has_value());
    }

    static void TestGetBusInfo() {
        trc::TransportCatalogue tc;

        tc.AddStop("A", {0, 0});
        tc.AddBus("a", {"A"});

        auto [bus_name, stop_count, unique_stop_count, route_distance,
              curvature] = tc.GetBusInfo("a").value();

        auto [bus_name_expected, stop_count_expected,
              unique_stop_count_expected, route_distance_expected] =
            tuple<string, size_t, size_t, double>{"a", 1, 1, 0.0};

        ASSERT_EQUAL(bus_name, bus_name_expected);
        ASSERT_EQUAL(stop_count, stop_count_expected);
        ASSERT_EQUAL(unique_stop_count, unique_stop_count_expected);
        ASSERT_EQUAL(route_distance, route_distance_expected);
        ASSERT(!tc.GetBusInfo("b").has_value());
    }

    static void TestConstructor() {
        vector<tuple<string, geo::Coordinates, unordered_map<string, double>>>
            stops_data = {
                {"A", {0, 0}, {}}, {"B", {1, 1}, {}}, {"C", {2, 2}, {}}};
        vector<tuple<string, vector<string>>> buses_data = {
            {"a", {"A", "B", "A"}}, {"b", {"C", "B", "C"}}};

        trc::TransportCatalogue tc(stops_data, buses_data);
        auto [stop_name1, coordinates1] = tc.GetStopProperties("A").value();
        auto [stop_name2, coordinates2] = tc.GetStopProperties("B").value();
        auto [stop_name3, coordinates3] = tc.GetStopProperties("C").value();

        auto [stop_name1_expected, coordinates1_expected] =
            tuple<string, geo::Coordinates>{"A", geo::Coordinates{0, 0}};
        auto [stop_name2_expected, coordinates2_expected] =
            tuple<string, geo::Coordinates>{"B", geo::Coordinates{1, 1}};
        auto [stop_name3_expected, coordinates3_expected] =
            tuple<string, geo::Coordinates>{"C", geo::Coordinates{2, 2}};

        ASSERT_EQUAL(stop_name1, stop_name1_expected);
        ASSERT_EQUAL(coordinates1, coordinates1_expected);

        ASSERT_EQUAL(stop_name2, stop_name2_expected);
        ASSERT_EQUAL(coordinates2, coordinates2_expected);

        ASSERT_EQUAL(stop_name3, stop_name3_expected);
        ASSERT_EQUAL(coordinates3, coordinates3_expected);

        auto [bus_name1, stop_count1, unique_stop_count1, route_distance1,
              curvature1] = tc.GetBusInfo("a").value();
        auto [bus_name2, stop_count2, unique_stop_count2, route_distance2,
              curvature2] = tc.GetBusInfo("b").value();

        auto [bus_name1_expected, stop_count1_expected,
              unique_stop_count1_expected, route_distance1_expected] =
            tuple<string, size_t, size_t, double>{"a", 3, 2, 0.0};
        auto [bus_name2_expected, stop_count2_expected,
              unique_stop_count2_expected, route_distance2_expected] =
            tuple<string, size_t, size_t, double>{"b", 3, 2, 0.0};

        ASSERT_EQUAL(bus_name1, bus_name1_expected);
        ASSERT_EQUAL(unique_stop_count1, unique_stop_count1_expected);
        ASSERT_EQUAL(stop_count1, stop_count1_expected);

        ASSERT_EQUAL(bus_name2, bus_name2_expected);
        ASSERT_EQUAL(unique_stop_count2, unique_stop_count2_expected);
        ASSERT_EQUAL(stop_count2, stop_count2_expected);
    }
};
#endif
}  // namespace test
}  // namespace trc
