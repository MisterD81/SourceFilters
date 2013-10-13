// Copyright 2012 MisterD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/** @file */

#ifndef _LOG4CPLUS_SPECIALFILE_APPENDER_HEADER_
#define _LOG4CPLUS_SPECIALFILE_APPENDER_HEADER_

#include <log4cplus/config.hxx>
#include <log4cplus/appender.h>
#include <log4cplus/fstreams.h>
#include <log4cplus/helpers/timehelper.h>
#include <log4cplus/helpers/lockfile.h>
#include <fstream>
#include <locale>
#include <memory>
#include <log4cplus/fileappender.h>

namespace log4cplus {

    /**
     * SpecialDailyRollingFileAppender extends {@link FileAppender} so that the
     * underlying file is rolled over at a user chosen frequency.
     *
     * <h3>Properties</h3>
     * <p>Properties additional to {@link FileAppender}'s properties:
     *
     * <dl>
     * <dt><tt>Schedule</tt></dt>
     * <dd>This property specifies rollover schedule. The possible
     * values are <tt>MONTHLY</tt>, <tt>WEEKLY</tt>, <tt>DAILY</tt>,
     * <tt>TWICE_DAILY</tt>, <tt>HOURLY</tt> and
     * <tt>MINUTELY</tt>.</dd>
     *
     * <dt><tt>MaxBackupIndex</tt></dt>
     * <dd>This property limits how many backup files are kept per
     * single logging period; e.g. how many <tt>log.2009-11-07.1</tt>,
     * <tt>log.2009-11-07.2</tt> etc. files are kept.</dd>
     *
     * </dl>
     */
    class LOG4CPLUS_EXPORT SpecialDailyRollingFileAppender : public FileAppender {
    public:
      // Ctors
        SpecialDailyRollingFileAppender(const log4cplus::tstring& filename,
                                 DailyRollingFileSchedule schedule = DAILY,
                                 bool immediateFlush = true,
                                 int maxBackupIndex = 10);
        SpecialDailyRollingFileAppender(const log4cplus::helpers::Properties& properties);

      // Dtor
        virtual ~SpecialDailyRollingFileAppender();
        
      // Methods
        virtual void close();

    protected:
        virtual void append(const spi::InternalLoggingEvent& event);
        void rollover();
        log4cplus::helpers::Time calculateNextRolloverTime(const log4cplus::helpers::Time& t) const;
        log4cplus::tstring getFilename(const log4cplus::helpers::Time& t) const;

      // Data
        DailyRollingFileSchedule schedule;
        log4cplus::tstring scheduledFilename;
        log4cplus::helpers::Time nextRolloverTime;
        int maxBackupIndex;

    private:
        void init(DailyRollingFileSchedule schedule);
    };

} // end namespace log4cplus

#endif // _LOG4CPLUS_SPECIALFILE_APPENDER_HEADER_

