#include "include/emio/detail/scan/float.hpp"
#include <iostream>
#include <string>
#include <iomanip>

int main() {
    using namespace emio::detail::scan;
    
    std::cout << std::fixed << std::setprecision(10);
    
    // Test basic decimal parsing
    std::cout << "=== Basic Decimal Tests ===" << std::endl;
    auto result1 = parse_float<double>("123.456");
    if (result1) {
        std::cout << "✓ Parsed 123.456 as: " << *result1 << std::endl;
    } else {
        std::cout << "✗ Failed to parse 123.456" << std::endl;
    }
    
    auto result1f = parse_float<float>("123.456");
    if (result1f) {
        std::cout << "✓ Parsed 123.456 as float: " << *result1f << std::endl;
    } else {
        std::cout << "✗ Failed to parse 123.456 as float" << std::endl;
    }
    
    // Test scientific notation
    std::cout << "\n=== Scientific Notation Tests ===" << std::endl;
    auto result2 = parse_float<double>("1.23e-4");
    if (result2) {
        std::cout << "✓ Parsed 1.23e-4 as: " << *result2 << std::endl;
    } else {
        std::cout << "✗ Failed to parse 1.23e-4" << std::endl;
    }
    
    auto result2b = parse_float<double>("5.67E+10");
    if (result2b) {
        std::cout << "✓ Parsed 5.67E+10 as: " << *result2b << std::endl;
    } else {
        std::cout << "✗ Failed to parse 5.67E+10" << std::endl;
    }
    
    // Test special values
    std::cout << "\n=== Special Values Tests ===" << std::endl;
    auto result3 = parse_float<double>("inf");
    if (result3) {
        std::cout << "✓ Parsed inf as: " << *result3 << std::endl;
    } else {
        std::cout << "✗ Failed to parse inf" << std::endl;
    }
    
    auto result3b = parse_float<double>("nan");
    if (result3b) {
        std::cout << "✓ Parsed nan as: " << *result3b << std::endl;
    } else {
        std::cout << "✗ Failed to parse nan" << std::endl;
    }
    
    auto result3c = parse_float<double>("infinity");
    if (result3c) {
        std::cout << "✓ Parsed infinity as: " << *result3c << std::endl;
    } else {
        std::cout << "✗ Failed to parse infinity" << std::endl;
    }
    
    // Test negative values
    std::cout << "\n=== Negative Values Tests ===" << std::endl;
    auto result4 = parse_float<double>("-42.5");
    if (result4) {
        std::cout << "✓ Parsed -42.5 as: " << *result4 << std::endl;
    } else {
        std::cout << "✗ Failed to parse -42.5" << std::endl;
    }
    
    auto result4b = parse_float<double>("-inf");
    if (result4b) {
        std::cout << "✓ Parsed -inf as: " << *result4b << std::endl;
    } else {
        std::cout << "✗ Failed to parse -inf" << std::endl;
    }
    
    // Test edge cases
    std::cout << "\n=== Edge Cases Tests ===" << std::endl;
    auto result5 = parse_float<double>("0.0");
    if (result5) {
        std::cout << "✓ Parsed 0.0 as: " << *result5 << std::endl;
    } else {
        std::cout << "✗ Failed to parse 0.0" << std::endl;
    }
    
    auto result6 = parse_float<double>("1e308");
    if (result6) {
        std::cout << "✓ Parsed 1e308 as: " << *result6 << std::endl;
    } else {
        std::cout << "✗ Failed to parse 1e308" << std::endl;
    }
    
    auto result7 = parse_float<double>("1e-308");
    if (result7) {
        std::cout << "✓ Parsed 1e-308 as: " << *result7 << std::endl;
    } else {
        std::cout << "✗ Failed to parse 1e-308" << std::endl;
    }
    
    // Test invalid cases
    std::cout << "\n=== Invalid Cases Tests ===" << std::endl;
    auto invalid1 = parse_float<double>("abc");
    if (!invalid1) {
        std::cout << "✓ Correctly rejected 'abc'" << std::endl;
    } else {
        std::cout << "✗ Incorrectly parsed 'abc' as: " << *invalid1 << std::endl;
    }
    
    auto invalid2 = parse_float<double>("123.456.789");
    if (!invalid2) {
        std::cout << "✓ Correctly rejected '123.456.789'" << std::endl;
    } else {
        std::cout << "✗ Incorrectly parsed '123.456.789' as: " << *invalid2 << std::endl;
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
