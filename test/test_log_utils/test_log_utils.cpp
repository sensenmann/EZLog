#include <Arduino.h>
#include <unity.h>

#define private public
#include "EZLog.h"
#undef private

void test_split_char_trims_whitespace() {
    const std::vector<std::string> tokens = EZLog::split("  alpha , beta ,gamma  ", ',');
    TEST_ASSERT_EQUAL_UINT32(3, tokens.size());
    TEST_ASSERT_EQUAL_STRING("alpha", tokens[0].c_str());
    TEST_ASSERT_EQUAL_STRING("beta", tokens[1].c_str());
    TEST_ASSERT_EQUAL_STRING("gamma", tokens[2].c_str());
}

void test_split_string_delimiter() {
    const std::vector<String> tokens = EZLog::split("one::two::three", "::");
    TEST_ASSERT_EQUAL_UINT32(3, tokens.size());
    TEST_ASSERT_EQUAL_STRING("one", tokens[0].c_str());
    TEST_ASSERT_EQUAL_STRING("two", tokens[1].c_str());
    TEST_ASSERT_EQUAL_STRING("three", tokens[2].c_str());
}

void test_trim_removes_outer_whitespace() {
    const String trimmed = EZLog::trim("\t  spaced text \n");
    TEST_ASSERT_EQUAL_STRING("spaced text", trimmed.c_str());
}

void test_format_number_adds_thousand_separators() {
    TEST_ASSERT_EQUAL_STRING("12", EZLog::formatNumber(12).c_str());
    TEST_ASSERT_EQUAL_STRING("1.234", EZLog::formatNumber(1234).c_str());
    TEST_ASSERT_EQUAL_STRING("1.234.567", EZLog::formatNumber(1234567).c_str());
}

void test_shouldLog_uses_custom_elements() {
    LoggingConfig config;
    config.loglevel = Loglevel::WARN;
    config.customLoggingElements = {
        LoggingElement("Net", Loglevel::DEBUG),
        LoggingElement("Root", std::vector<LoggingElement>{
            LoggingElement("Service::Core", Loglevel::DEBUG),
        }),
    };

    EZLog::init(config);

    TEST_ASSERT_TRUE(EZLog::_shouldLog("Net::Client", Loglevel::INFO));
    TEST_ASSERT_FALSE(EZLog::_shouldLog("Net::Client", Loglevel::VERBOSE));
    TEST_ASSERT_TRUE(EZLog::_shouldLog("Service::Core::Init", Loglevel::DEBUG));
    TEST_ASSERT_FALSE(EZLog::_shouldLog("Service::Core::Init", Loglevel::VERBOSE));
}

void test_shouldLog_falls_back_to_default_log_level() {
    LoggingConfig config;
    config.loglevel = Loglevel::WARN;
    config.customLoggingElements.clear();

    EZLog::init(config);

    TEST_ASSERT_TRUE(EZLog::_shouldLog("Unknown::Thing", Loglevel::WARN));
    TEST_ASSERT_FALSE(EZLog::_shouldLog("Unknown::Thing", Loglevel::INFO));
}

void test_shouldLog_override_logs_everything() {
    LoggingConfig config;
    config.overrideLogAll = true;
    config.loglevel = Loglevel::ERROR;

    EZLog::init(config);

    TEST_ASSERT_TRUE(EZLog::_shouldLog("Anything", Loglevel::VERBOSE));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_split_char_trims_whitespace);
    RUN_TEST(test_split_string_delimiter);
    RUN_TEST(test_trim_removes_outer_whitespace);
    RUN_TEST(test_format_number_adds_thousand_separators);
    RUN_TEST(test_shouldLog_uses_custom_elements);
    RUN_TEST(test_shouldLog_falls_back_to_default_log_level);
    RUN_TEST(test_shouldLog_override_logs_everything);
    UNITY_END();
}

void loop() {}
