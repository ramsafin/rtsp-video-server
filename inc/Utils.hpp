#ifndef LIVE_VIDEO_STREAM_UTILS_HPP
#define LIVE_VIDEO_STREAM_UTILS_HPP

#include <string>
#include <initializer_list>

namespace LIRS {

    /**
     * Utility methods.
     */

    namespace utils {

        /**
         * Concatenates specified arguments using delimiter/tail.
         *
         * @param args elements to be concatenated.
         * @param delimiter string used between concatenated arguments.
         * @param tail string concatenated at the end.
         * @return resulting concatenated string.
         */
        std::string concatParams(std::initializer_list<size_t> args, std::string delimiter = {}, std::string tail = {});

        std::string to_string_with_prefix(size_t val, std::string prefix = {});
    }
}

#endif //LIVE_VIDEO_STREAM_UTILS_HPP
