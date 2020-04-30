#include <catch2/catch.hpp>
#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include <climits>


constexpr uint32_t InvalidPrice = UINT32_MAX;

struct Quote
{
    uint32_t px  = InvalidPrice;
    uint32_t qty = 0;
};

enum class Action
{
    eInvalid,
    eUpdate,
    eCancel,
    eAdd,
};

struct Diff
{
    Action   action;
    uint32_t px;
    uint32_t qty;
};

const char* action_name(Action x)
{
    switch (x) {
        case Action::eInvalid: return "Invalid";
        case Action::eUpdate:  return "Update";
        case Action::eCancel:  return "Cancel";
        case Action::eAdd:     return "Add";
    }
    return "Unknown";
}

constexpr int MaxPerSide = 5;
using Quotes = std::array<Quote, MaxPerSide>;
// using Diffs  = std::array<Diff,  2*MaxPerSide>;
using Diffs  = std::vector<Diff>;

bool operator==(const Quote& lhs, const Quote& rhs)
{
    return lhs.px == rhs.px && lhs.qty == rhs.qty;
}

bool operator!=(const Quote& lhs, const Quote& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Quote& q) {
    os << "(" << q.px << ", " << q.qty << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Quotes& quotes) {
    os << "[ ";
    for (const auto& q : quotes) {
        os << q << " ";
    }
    os << "]";
    return os;
}

bool operator==(const Quotes& lhs, const Quotes& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const Quotes& lhs, const Quotes& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Diff& d)
{
    os << "(" << action_name(d.action) << ", px=" << d.px << ", qty=" << d.qty << ")";
    return os;
}

bool operator==(const Diff& lhs, const Diff& rhs)
{
    return lhs.action == rhs.action && lhs.px == rhs.px && lhs.qty == rhs.qty;
}

bool operator!=(const Diff& lhs, const Diff& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Diffs& diffs) {
    os << "[ ";
    for (const auto& d : diffs) {
        os << d << " ";
    }
    os << "]";
    return os;
}

bool operator==(const Diffs& lhs, const Diffs& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const Diffs& lhs, const Diffs& rhs) {
    return !(lhs == rhs);
}

struct QuotePriceCmp
{
    bool operator()(const Quote& a, const Quote& b) noexcept {
        return a.px < b.px;
    }
};

struct QuoteCmp
{
    bool operator()(const Quote& a, const Quote& b) noexcept {
        return a.px == b.px && a.qty == b.qty;
    }
};

Quotes apply_diffs(const Quotes& current, const Diffs& diffs) {
    // Quotes result = current;
    std::vector<Quote> qs;
    for (const auto& q : current) {
        if (q.qty > 0) {
            qs.push_back(q);
        }
    }
    for (const auto& diff_ : diffs) {
        if (diff_.action == Action::eInvalid) {
            throw std::runtime_error("invalid diff!");
        }
        else if (diff_.action == Action::eCancel) {
            auto px = diff_.px;
            auto it = std::find_if(std::begin(qs), std::end(qs), [px](const Quote& q) { return q.px == px; });
            if (it == std::end(qs)) {
                throw std::runtime_error("invalid diff: canceling non-existent price");
            }
            // this isn't strictly necessary:
            if (it->qty != diff_.qty) {
                throw std::runtime_error("invalid diff: canceling price, but sizes do not match");
            }
            qs.erase(it);
            // it->px  = InvalidPrice;
            // it->qty = 0u;
        }
        else if (diff_.action == Action::eAdd) {
            qs.push_back(Quote{diff_.px, diff_.qty});
        }
        else if (diff_.action == Action::eUpdate) {
            auto px = diff_.px;
            auto it = std::find_if(std::begin(qs), std::end(qs), [px](const Quote& q) { return q.px == px; });
            if (it == std::end(qs)) {
                throw std::runtime_error("invalid diff: updating non-existent price");
            }
            it->qty = diff_.qty;
        }
        else {
            throw std::runtime_error("invalid diff: unknown action");
        }
    }
    Quotes result;
    REQUIRE(qs.size() <= result.size());
    std::fill(std::begin(result), std::end(result), Quote{});
    for (std::size_t i = 0; i < qs.size(); ++i) {
        result[i] = qs[i];
    }
    std::sort(std::begin(result), std::end(result), QuotePriceCmp{});
    return result;
}

Diffs diff(const Quotes& current, Quotes& desired)
{
    // std::sort(std::begin(current), std::end(current), QuotePriceCmp{});
    std::sort(std::begin(desired), std::end(desired), QuotePriceCmp{});

    REQUIRE(std::is_sorted(std::begin(current), std::end(current), QuotePriceCmp{}));
    REQUIRE(std::is_sorted(std::begin(desired), std::end(desired), QuotePriceCmp{}));

    Diffs diffs;
    std::size_t idx1 = 0;
    std::size_t idx2 = 0;
    while (1) {
        if (!(idx1 < current.size())) {
            break;
        }
        if (!(idx2 < desired.size())) {
            break;
        }
        if (current[idx1].qty == 0u) {
            break;
        }
        if (desired[idx2].qty == 0u) {
            break;
        }

        if (current[idx1].px == desired[idx2].px) {                     // curent.px == desired.px
            if (current[idx1].qty != desired[idx2].qty) {
                Diff diff;
                diff.action = Action::eUpdate;
                diff.px  = current[idx1].px;
                diff.qty = desired[idx2].qty;
                diffs.push_back(diff);
            }
            ++idx1;
            ++idx2;
            continue;
        }
        else if (QuotePriceCmp{}(current[idx1], desired[idx2])) { // current.px < desired.px
            // TODO: slightly suboptimal, potentially could modify into next quote
            Diff diff;
            diff.action = Action::eCancel;
            diff.px     = current[idx1].px;
            diff.qty    = current[idx1].qty;
            // diff.qty    = 0u;
            diffs.push_back(diff);
            ++idx1;
            continue;
        } else {
            assert(!QuotePriceCmp{}(current[idx1], desired[idx2])); // current.px >= desired.px
            Diff diff;
            diff.action = Action::eAdd;
            diff.px     = desired[idx2].px;
            diff.qty    = desired[idx2].qty;
            diffs.push_back(diff);
            ++idx2;
        }
    }

    while (idx1 < current.size() && current[idx1].qty != 0u) {
        Diff diff;
        diff.action = Action::eCancel;
        diff.px     = current[idx1].px;
        diff.qty    = current[idx1].qty;
        diffs.push_back(diff);
        idx1++;
    }

    while (idx2 < desired.size() && desired[idx2].qty != 0u) {
        Diff diff;
        diff.action = Action::eAdd;
        diff.px     = desired[idx2].px;
        diff.qty    = desired[idx2].qty;
        diffs.push_back(diff);
        idx2++;
    }

    return diffs;
}

TEST_CASE("Same quote set should create no diffs")
{
    Quotes current = { Quote{1000, 1}, Quote{1001, 1} , Quote{1002, 1} };
    Quotes desired = { Quote{1000, 1}, Quote{1001, 1} , Quote{1002, 1} };
    Diffs diffs = diff(current, desired);
    REQUIRE(diffs.empty());
}

TEST_CASE("Add one new price")
{
    Quotes current = { Quote{1000, 1}, Quote{1002, 1} };

    SECTION("Add new lowest price")
    {
        Quotes desired = { Quote{999, 1}, Quote{1000, 1}, Quote{1002, 1} };
        Diffs  expect = Diffs{ Diff{Action::eAdd, 999, 1} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }

    SECTION("Add new highest price")
    {
        Quotes desired = { Quote{1000, 1}, Quote{1002, 1}, Quote{1003, 1} };
        Diffs  expect = Diffs{ Diff{Action::eAdd, 1003, 1} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }

    SECTION("Add new middle price")
    {
        Quotes desired = { Quote{1000, 1}, Quote{1001, 1}, Quote{1002, 1} };
        Diffs  expect = Diffs{ Diff{Action::eAdd, 1001, 1} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }
}

TEST_CASE("Delete one price")
{
    Quotes current = { Quote{1000, 1}, Quote{1002, 1}, Quote{1004, 2} };

    SECTION("Delete lowest price")
    {
        Quotes desired = {                 Quote{1002, 1}, Quote{1004, 2} };
        Diffs  expect = Diffs{ Diff{Action::eCancel, 1000, 1} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }

    SECTION("Delete middle price")
    {
        Quotes desired = { Quote{1000, 1},                 Quote{1004, 2} };
        Diffs  expect = Diffs{ Diff{Action::eCancel, 1002, 1} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }

    SECTION("Delete highest price")
    {
        Quotes desired = { Quote{1000, 1}, Quote{1002, 1}                 };
        Diffs  expect = Diffs{ Diff{Action::eCancel, 1004, 2} };
        auto   result = diff(current, desired);
        CHECK(result == expect);
        auto   after = apply_diffs(current, result);
        CHECK(after == desired);
    }
}
