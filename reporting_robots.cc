#include "reporting_robots.h"

#include <algorithm>
#include <string>
#include <vector>

#include "absl/base/macros.h"
#include "absl/log/check.h"

namespace googlebot {
// The kUnsupportedTags tags are popular tags in robots.txt files, but Google
// doesn't use them for anything. Other search engines may, however, so we
// parse them out so users of the library can highlight them for their own
// users if they so wish.
// These are different from the "unknown" tags, since we know that these may
// have some use cases; to the best of our knowledge other tags we find, don't.
// (for example, "unicorn" from "unicorn: /value")
static const std::vector<std::string> kUnsupportedTags = {
    "clean-param", "crawl-delay", "host", "noarchive", "noindex", "nofollow"};

void RobotsParsingReporter::Digest(
    int line_num,
    RobotsParsingReporter::RobotsParsedLine::RobotsTagName parsed_tag,
    bool is_typo) {
  ABSL_ASSERT(line_num > last_line_seen_);
  last_line_seen_ = line_num;
  if (parsed_tag != RobotsParsedLine::kUnknown &&
      parsed_tag != RobotsParsedLine::kUnused) {
    ++valid_directives_;
  }

  RobotsParsingReporter::RobotsParsedLine plr;
  plr.line_num = line_num;
  plr.is_typo = is_typo;
  plr.tag_name = parsed_tag;
  robots_parse_results_.push_back(plr);
}

void RobotsParsingReporter::HandleRobotsStart() {
  last_line_seen_ = 0;
  valid_directives_ = 0;
  unused_directives_ = 0;
}
void RobotsParsingReporter::HandleRobotsEnd() {}
void RobotsParsingReporter::HandleUserAgent(int line_num,
                                            absl::string_view line_value) {
  Digest(line_num, RobotsParsedLine::kUserAgent,
         false /* typo info not yet available */);
}
void RobotsParsingReporter::HandleAllow(int line_num,
                                        absl::string_view line_value) {
  Digest(line_num, RobotsParsingReporter::RobotsParsedLine::kAllow,
         false /* we don't accept typos for sallow keys */);
}
void RobotsParsingReporter::HandleDisallow(int line_num,
                                           absl::string_view line_value) {
  Digest(line_num, RobotsParsingReporter::RobotsParsedLine::kDisallow,
         false /* typo info not yet available */);
}
void RobotsParsingReporter::HandleSitemap(int line_num,
                                          absl::string_view line_value) {
  Digest(line_num, RobotsParsingReporter::RobotsParsedLine::kSitemap,
         false /* we don't accept typos for sitemap keys */);
}
void RobotsParsingReporter::HandleUnknownAction(int line_num,
                                                absl::string_view action,
                                                absl::string_view line_value) {
  RobotsParsingReporter::RobotsParsedLine::RobotsTagName rtn =
      std::count(kUnsupportedTags.begin(), kUnsupportedTags.end(), action) > 0
          ? RobotsParsingReporter::RobotsParsedLine::kUnused
          : RobotsParsingReporter::RobotsParsedLine::kUnknown;
  unused_directives_++;
  Digest(line_num, rtn, false /* by definition these can't be typos */);
}

}  // namespace googlebot
