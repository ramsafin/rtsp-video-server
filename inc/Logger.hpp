#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <unistd.h>

#ifdef HAVE_LOG4CPP

#include "log4cpp/Category.hh"
#include "log4cpp/RollingFileAppender.hh"
#include "log4cpp/PatternLayout.hh"

#define LOG(__level)  log4cpp::Category::getRoot() << log4cpp::Priority::__level << __FILE__ << ":" << __LINE__ << "\n\t"

constexpr static size_t MAX_FILE_SIZE = 100 * 1024 * 1024;

constexpr static size_t MAX_BACKUP_INDEX = 5;

inline void initLogger(log4cpp::Priority::PriorityLevel level) {

    // initialize log4cpp
    log4cpp::Category &rootCategory = log4cpp::Category::getRoot();

//    log4cpp::Appender *rolling = new log4cpp::RollingFileAppender("root", "server.log", MAX_FILE_SIZE, MAX_BACKUP_INDEX);
    log4cpp::Appender *console = new log4cpp::FileAppender("root", fileno(stderr));

    auto patterLayout = new log4cpp::PatternLayout();
    patterLayout->setConversionPattern("%d [%-6p] - %m%n");

//    rolling->setLayout(patterLayout);
    console->setLayout(patterLayout);

//    rootCategory.addAppender(rolling);
    rootCategory.addAppender(console);

    rootCategory.setPriority(level);
}

#endif

#endif // LOGGER_HPP

