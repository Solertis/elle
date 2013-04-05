#define BOOST_TEST_DYN_LINK

#include <sstream>
#include <thread>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>

#include <elle/log/Logger.cc>
#include <elle/log/TextLogger.hh>
#include <elle/os/getenv.hh>
#include <elle/os/setenv.hh>
#include <elle/os/unsetenv.hh>

static
void
plain_indent_test()
{
  elle::log::PlainIndentation indenter;
  indenter.indent();
  indenter.indent();
  BOOST_CHECK_EQUAL(indenter.indentation(), 2);
  indenter.unindent();
  BOOST_CHECK_EQUAL(indenter.indentation(), 1);
}

static
void
logger_indent_test()
{
  std::stringstream ss;
  elle::log::TextLogger* logger = new elle::log::TextLogger(ss);

  for (int i=0; i<5; i++)
    logger->indent();

  std::thread t1(std::bind(&elle::log::TextLogger::indent, logger));
  std::thread t2(std::bind(&elle::log::TextLogger::indent, logger));
  std::thread t3(std::bind(&elle::log::TextLogger::unindent, logger));
  std::thread t4(std::bind(&elle::log::TextLogger::unindent, logger));
  std::thread t5(std::bind(&elle::log::TextLogger::indent, logger));
  std::thread t6(std::bind(&elle::log::TextLogger::unindent, logger));
  std::thread t7(std::bind(&elle::log::TextLogger::indent, logger));
  std::thread t8(std::bind(&elle::log::TextLogger::unindent, logger));
  std::thread t9(std::bind(&elle::log::TextLogger::indent, logger));
  std::thread t10(std::bind(&elle::log::TextLogger::unindent, logger));

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  t7.join();
  t8.join();
  t9.join();
  t10.join();

  BOOST_CHECK_EQUAL(logger->indentation(), 5);
  delete logger;
}

static
void
message_test()
{
  std::stringstream ss;
  elle::log::TextLogger* logger = new elle::log::TextLogger(ss);

  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);

  logger->indent();
  logger->message(elle::log::Logger::Level::log,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Test Message");
  BOOST_CHECK_EQUAL(ss.str(), "[Test] Test Message\n");

  ss.str("");
  logger->indent();
  logger->message(elle::log::Logger::Level::log,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Another Test Message");
  BOOST_CHECK_EQUAL(ss.str(), "[Test]   Another Test Message\n");

  logger->unindent();
  ss.str("");
  BOOST_CHECK_EQUAL(logger->component_enabled("Another"), true);
  logger->message(elle::log::Logger::Level::log,
                  elle::log::Logger::Type::info,
                  "Another",
                  "Test");
  BOOST_CHECK_EQUAL(ss.str(), "[Another] Test\n");

  ss.str("");
  logger->message(elle::log::Logger::Level::trace,
                  elle::log::Logger::Type::info,
                  "Another",
                  "Test2");
  BOOST_CHECK_EQUAL(ss.str(), "[Another] Test2\n");

  ss.str("");
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::info,
                  "Another",
                  "Test3");
  BOOST_CHECK_EQUAL(ss.str(), "[Another] Test3\n");

  ss.str("");
  logger->message(elle::log::Logger::Level::dump,
                  elle::log::Logger::Type::info,
                  "Another",
                  "Test4");
  BOOST_CHECK_EQUAL(ss.str(), "[Another] Test4\n");

  ss.str("");
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::warning,
                  "Test",
                  "Test5");
  BOOST_CHECK_EQUAL(ss.str(), "[33;01;33m[ Test  ] Test5\n[0m");

  ss.str("");
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::error,
                  "Test",
                  "Test6");
  BOOST_CHECK_EQUAL(ss.str(), "[33;01;31m[ Test  ] Test6\n[0m");

  delete logger;
}

static
void
clear_env()
{
  elle::os::unsetenv("ELLE_LOG_TIME", "");
  elle::os::unsetenv("ELLE_LOG_TIME_UNIVERSAL", "");
  elle::os::unsetenv("ELLE_LOG_PID", "");
}

static
void
environment_format_test()
{
  std::stringstream ss, res;
  elle::log::TextLogger* logger;

  ss.str("");
  res.str("");
  clear_env();
  elle::os::setenv("ELLE_LOG_TIME", "1", 0);
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME_UNIVERSAL", ""), "");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_PID", ""), "");
  logger = new elle::log::TextLogger(ss);
  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);
  logger->indent();
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Test");
  res << boost::posix_time::second_clock::local_time()
      << ": [Test] Test\n";
  BOOST_CHECK_EQUAL(ss.str(), res.str());
  delete logger;

  ss.str("");
  res.str("");
  clear_env();
  elle::os::setenv("ELLE_LOG_TIME", "1", 0);
  elle::os::setenv("ELLE_LOG_TIME_UNIVERSAL", "1", 0);
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME_UNIVERSAL"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_PID", ""), "");
  logger = new elle::log::TextLogger(ss);
  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);
  logger->indent();
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Test 2");
  res << boost::posix_time::second_clock::universal_time() << ": "
      << "[Test] Test 2\n";
  BOOST_CHECK_EQUAL(ss.str(), res.str());
  delete logger;

  ss.str("");
  res.str("");
  clear_env();
  elle::os::setenv("ELLE_LOG_PID", "1", 0);
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME", ""), "");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME_UNIVERSAL", ""), "");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_PID"), "1");
  logger = new elle::log::TextLogger(ss);
  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);
  logger->indent();
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Test 3");
  res << "[" << boost::lexical_cast<std::string>(getpid()) << "] "
      << "[Test] Test 3\n";
  BOOST_CHECK_EQUAL(ss.str(), res.str());
  delete logger;

  ss.str("");
  res.str("");
  clear_env();
  elle::os::setenv("ELLE_LOG_TIME", "1", 0);
  elle::os::setenv("ELLE_LOG_TIME_UNIVERSAL", "1", 0);
  elle::os::setenv("ELLE_LOG_PID", "1", 0);
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME_UNIVERSAL"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_PID"), "1");
  logger = new elle::log::TextLogger(ss);
  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);
  logger->indent();
  logger->message(elle::log::Logger::Level::debug,
                  elle::log::Logger::Type::info,
                  "Test",
                  "Test 4");
  res << boost::posix_time::second_clock::universal_time() << ": "
      << "[" << boost::lexical_cast<std::string>(getpid()) << "] "
      << "[Test] Test 4\n";
  BOOST_CHECK_EQUAL(ss.str(), res.str());
  delete logger;

  ss.str("");
  res.str("");
  clear_env();
  elle::os::setenv("ELLE_LOG_TIME", "1", 0);
  elle::os::setenv("ELLE_LOG_PID", "1", 0);
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME"), "1");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_TIME_UNIVERSAL", ""), "");
  BOOST_CHECK_EQUAL(elle::os::getenv("ELLE_LOG_PID"), "1");
  logger = new elle::log::TextLogger(ss);
  BOOST_CHECK_EQUAL(logger->component_enabled("Test"), true);
  logger->indent();
  logger->message(elle::log::Logger::Level::dump,
                  elle::log::Logger::Type::warning,
                  "Test",
                  "Test 5");
  res << "[33;01;33m"
      << boost::posix_time::second_clock::local_time() << ": "
      << "[" << boost::lexical_cast<std::string>(getpid()) << "] "
      << "[Test] Test 5\n[0m";
  BOOST_CHECK_EQUAL(ss.str(), res.str());
  delete logger;

}

static
void
parallel_write()
{
  std::stringstream output;
  elle::log::TextLogger logger(output);
  logger.component_enabled("in");
  logger.component_enabled("out");

  auto action = [&logger](int& counter)
    {
      using namespace boost::posix_time;
      ptime deadline = microsec_clock::local_time() + seconds(1);
      while (microsec_clock::local_time() < deadline)
      {
        logger.indent();
        logger.message(elle::log::Logger::Level::log,
                       elle::log::Logger::Type::info,
                       "out",
                       "out");
        logger.indent();
        logger.message(elle::log::Logger::Level::log,
                       elle::log::Logger::Type::error,
                       "in",
                       "in");
        logger.unindent();
        logger.unindent();
        ++counter;
      }
    };

  int c1 = 0;;
  std::thread t1([&](){ action(c1); });
  int c2 = 0;
  std::thread t2([&](){ action(c2); });

  t1.join();
  t2.join();

  BOOST_CHECK_GT(c1, 64);
  BOOST_CHECK_GT(c2, 64);
}

static
bool
test_suite()
{
  boost::unit_test::test_suite* indentation = BOOST_TEST_SUITE("Indentation");
  boost::unit_test::framework::master_test_suite().add(indentation);
  indentation->add(BOOST_TEST_CASE(std::bind(plain_indent_test)));

  boost::unit_test::test_suite* logger = BOOST_TEST_SUITE("Logger");
  boost::unit_test::framework::master_test_suite().add(logger);
  logger->add(BOOST_TEST_CASE(std::bind(logger_indent_test)));
  logger->add(BOOST_TEST_CASE(std::bind(message_test)));
  logger->add(BOOST_TEST_CASE(std::bind(environment_format_test)));

  boost::unit_test::test_suite* concurrency = BOOST_TEST_SUITE("Concurrency");
  boost::unit_test::framework::master_test_suite().add(concurrency);
  concurrency->add(BOOST_TEST_CASE(std::bind(parallel_write)));

  return true;
}

int
main(int argc, char** argv)
{
  elle::os::setenv("ELLE_LOG_COLOR", "1", 0);
  return ::boost::unit_test::unit_test_main(test_suite, argc, argv);
}
