/// @file       LiveTraffic.h
/// @brief      Umbrella header file, including all others; defines global functions mainly implemented in `LTMain.cpp`
/// @details    All necessary header files: Standard C, Windows, Open GL, C++, X-Plane, libxplanemp and other libs, LTAPI, LiveTraffic\n
///             To be used for header pre-processing\n
///             Set of `LTMain...` functions, which control initialization and shutdown
///             Global utility functions: path helpers, opening URLs, string helpers\n
/// @author     Birger Hoppe
/// @copyright  (c) 2018-2020 Birger Hoppe
/// @copyright  Permission is hereby granted, free of charge, to any person obtaining a
///             copy of this software and associated documentation files (the "Software"),
///             to deal in the Software without restriction, including without limitation
///             the rights to use, copy, modify, merge, publish, distribute, sublicense,
///             and/or sell copies of the Software, and to permit persons to whom the
///             Software is furnished to do so, subject to the following conditions:\n
///             The above copyright notice and this permission notice shall be included in
///             all copies or substantial portions of the Software.\n
///             THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///             IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///             FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///             AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///             LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///             OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///             THE SOFTWARE.

#ifndef LiveTraffic_h
#define LiveTraffic_h

// LiveTraffic is coded against SDK 3.01 (X-Plane 11.20 and above), so XPLM200, XPLM210, XPLM300, and XPLM301 must be defined
// XP10 compatibility is achieved by not using XP11 functions directly, see XPCompatibility.cpp.
// By defining up to XPLM301 all data types are available already. There are only very very rare cases when a structure
// gets extended in XPLM300 and later. Those rare cases are covered in code (see TextIO.cpp for an example).
#if !defined(XPLM200) || !defined(XPLM210) || !defined(XPLM300) || !defined(XPLM301)
#error This is made to be compiled at least against the XPLM301 SDK (X-plane 11.20 and above)
#endif

// MARK: Includes
// Standard C
#include <sys/stat.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cassert>
#include <locale>
#if APL
#include <xlocale.h>
#endif

// Windows
#if IBM
#include <winsock2.h>
#include <windows.h>
#include <processthreadsapi.h>
// we prefer std::max/min of <algorithm>
#undef max
#undef min
#endif

// Open GL
#include "SystemGL.h"

// C++
#include <climits>
#include <utility>
#include <string>
#include <array>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <thread>
#include <future>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <chrono>
#include <regex>

// X-Plane SDK
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMCamera.h"
#include "XPLMNavigation.h"

// Base64
#include "base64.h"

// ImGui / ImgWindow
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"         // For fiddling with the ImGui window z-order we need ImGui internals
#include "ImgWindow.h"
#include "ImgFontAtlas.h"
#include "IconsFontAwesome5.h"

// XP Multiplayer API (XPMP2)
#define INCLUDE_FMOD_SOUND 1            // Built with FMOD sound support
#include "XPMPMultiplayer.h"
#include "XPMPAircraft.h"

// FMOD Logo
#include "FMOD_Logo.h"

// LTAPI Includes, this defines the bulk transfer structure
#include "LTAPI.h"

// LiveTraffic Includes
#include "Constants.h"
#include "DataRefs.h"

// Global DataRef object, which also includes 'global' variables
extern DataRefs dataRefs;

#include "CoordCalc.h"
#include "TextIO.h"
#include "LTFlightData.h"
#include "LTAircraft.h"
#include "LTImgWindow.h"
#include "SettingsUI.h"
#include "ACInfoWnd.h"
#include "ACTable.h"
#include "InfoListWnd.h"
#include "LTApt.h"
#include "LTWeather.h"

// LiveTraffic channels
#include "../Lib/XPMP2/src/Network.h"
#include "LTChannel.h"
#include "LTForeFlight.h"
#include "LTRealTraffic.h"
#include "LTOpenSky.h"
#include "LTADSBEx.h"
#include "LTADSBHub.h"
#include "LTOpenGlider.h"
#include "LTFSCharter.h"
#include "LTSynthetic.h"
#include "LTFlightRadar.h"

//MARK: Global Control functions
bool LTMainInit ();
bool LTMainEnable ();
bool LTMainShowAircraft ();
bool LTMainTryGetAIAircraft ();
void LTMainReleaseAIAircraft ();
void LTMainToggleAI (bool bGetControl);
void LTMainHideAircraft ();
void LTMainDisable ();
void LTMainStop ();

void LTRegularUpdates();        ///< collects all updates that need to be done up to every flight loop cycle
void MenuUpdateAllItemStatus();
void HandleNewVersionAvail ();

#ifdef DEBUG
void LTErrorCB (const char* msg);
#endif

// MARK: Path helpers

// deal with paths: make a full one from a relative one or keep a full path
std::string LTCalcFullPath ( const std::string& path );
std::string LTCalcFullPluginPath ( const std::string& path );

/// returns path, with XP system path stripped if path starts with it
std::string LTRemoveXPSystemPath (const std::string& path);
/// strips XP system path if path starts with it
void LTRemoveXPSystemPath (std::string& path);

// given a path (in XPLM notation) returns number of files in the path
// or 0 in case of errors
int LTNumFilesInPath ( const std::string& path );

/// Is path a directory?
bool IsDir (const std::string& path);

/// List of files in a directory (wrapper around XPLMGetDirectoryContents)
std::vector<std::string> GetDirContents (const std::string& path, bool bDirOnly = false);

/// @brief Read a text line from file, no matter if ended by CRLF or LF
std::istream& safeGetline(std::istream& is, std::string& t);

/// Get file's modification time (0 in case of errors)
time_t GetFileModTime(const std::string& path);

/// @brief Lookup a record by key in a sorted binary record-based file
/// @param f File to search, must have been opened in binary input mode
/// @param[in,out] n File size in number of records, will be determined and returned if `0`
/// @param key Key to find, expected to be at the record's beginning
/// @param[in,out] minKey is the lowest key in the file (record 0)
/// @param[in,out] maxKey is the highest key in the file (last record), determined if `0`
/// @param[out] outRec points to an output buffer, which is used as temporary and in the end contains the found record
/// @param recLen Length of each record and (minimum) size of the buffer `outRec` points to
/// @see https://en.wikipedia.org/wiki/Binary_search_algorithm
/// @details Linear interpolation is applied to the key
bool FileRecLookup (std::ifstream& f, size_t& n,
                    unsigned long key,
                    unsigned long& minKey, unsigned long& maxKey,
                    void* outRec, size_t recLen);

// MARK: URL/Help support

void LTOpenURL  (const std::string& url, const std::string& addon = "");
void LTOpenHelp (const std::string& path);

// MARK: Remote File Download

/// Download the given file, `false` if HTTP 404 not found, exceptions otherwise
bool RemoteFileDownload (const std::string& url, const std::string& path);

// MARK: String/Text Functions

// change a std::string to uppercase
std::string& str_toupper(std::string& s);
/// return a std::string copy converted to uppercase
std::string str_toupper_c(const std::string& s);
/// Case-insensitive equal
bool striequal (const std::string& a, const std::string& b);
/// Case-insensitive begins with
bool stribeginwith (const std::string& s, const std::string& begin);
// are all chars alphanumeric?
bool str_isalnum(const std::string& s);
// limits text to m characters, replacing the last ones with ... if too long
inline std::string strAtMost(const std::string s, size_t m) {
    return s.length() <= m ? s :
    s.substr(0, m-3) + "...";
}

/// Replace all occurences of one string with another
void str_replaceAll(std::string& str, const std::string& from, const std::string& to, size_t start_pos = 0);

/// @brief Replace a potentially wrong decimal point
/// @returns if `true` if locale defines other decimal point than `.`
bool str_correctDecimalPt (std::string& str, size_t start_pos = 0);

// trimming of string
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// trim from end of string (right)
inline std::string& rtrim(std::string& s, const char* t = WHITESPACE)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}
// trim from beginning of string (left)
inline std::string& ltrim(std::string& s, const char* t = WHITESPACE)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}
// trim from both ends of string (right then left)
inline std::string& trim(std::string& s, const char* t = WHITESPACE)
{
    return ltrim(rtrim(s, t), t);
}

/// trim whitespace
inline std::string& trim_ws(std::string& s) { return trim(s); }

/// Cut off everything after `from` from `s`, `from` including
std::string& cut_off(std::string& s, const std::string& from);

// last word of a string
std::string str_last_word (const std::string& s);
// separates string into tokens
std::vector<std::string> str_tokenize (const std::string& s,
                                       const std::string& tokens,
                                       bool bSkipEmpty = true);
/// concatenates a vector of strings into one string (reverse of str_tokenize)
std::string str_concat (const std::vector<std::string>& vs, const std::string& separator);
// returns first non-empty string, and "" in case all are empty
std::string str_first_non_empty (const std::initializer_list<const std::string>& l);

// separate string into fields with a multi-character delimiter
std::vector<std::string> str_fields (const std::string& s,
                                     const std::string& delim);

/// Replaces personal information in the string, like email address
std::string& str_replPers (std::string& s);

// push a new item to the end only if it doesn't exist yet
template< class ContainerT>
void push_back_unique(ContainerT& list, typename ContainerT::const_reference key)
{
    if ( std::find(list.cbegin(),list.cend(),key) == list.cend() )
        list.push_back(key);
}

/// Base64 encoding
std::string EncodeBase64 (const std::string& _clear);
/// Base64 decoding
std::string DecodeBase64 (const std::string& _encoded);
/// XOR a string s with another one t, potentially repeating the application of t if t is shorter than s
std::string str_xor (const std::string& s, const char* t);
/// Obfuscate a secret string for storing in the settings file
std::string Obfuscate (const std::string& _clear);
/// Undo obfuscation
std::string Cleartext (const std::string& _obfuscated);

// MARK: Time Functions

/// System time in seconds with fractionals
inline double GetSysTime ()
{   return
    // system time in microseconds
    double(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
    // divided by 1000000 to create seconds with fractionals
    / 1000000.0; }

/// Returns timezone difference between local and GMT in seconds
int timeOffsetUTC();

/// Converts date/time (UTC) to epoch value
inline time_t mktime_utc (std::tm& tm)
{ return mktime(&tm) + timeOffsetUTC(); }

/// Converts a UTC time to epoch value, assuming today's date
time_t mktime_utc (int h, int min, int s);

/// Convert time string "YYYY-MM-DD HH:MM:SS" to epoch value
time_t mktime_string (const std::string& s);

// format timestamp
std::string ts2string (time_t t);

/// Converts an epoch timestamp to a Zulu time string incl. 10th of seconds
std::string ts2string (double _zt, int secDecimals=1);

/// Convert an XP network time float to a string
std::string NetwTimeString (float _runS);

/// @brief Convenience function to check on something at most every x seconds
/// @param _lastCheck Provide a float which holds the time of last check (init with `0.0f`)
/// @param _interval [seconds] How often to perform the check?
/// @param _now Current time, possibly from a call to GetTotalRunningTime()
/// @return `true` if more than `_interval` time has passed since `_lastCheck`
bool CheckEverySoOften (float& _lastCheck, float _interval, float _now);

/// @brief Convenience function to check on something at most every x seconds
/// @param _lastCheck Provide a float which holds the time of last check (init with `0.0f`)
/// @param _interval [seconds] How often to perform the check?
/// @return `true` if more than `_interval` time has passed since `_lastCheck`
inline bool CheckEverySoOften (float& _lastCheck, float _interval)
{ return CheckEverySoOften(_lastCheck, _interval, dataRefs.GetMiscNetwTime()); }

// MARK: Other Utility Functions

/// Convert barometric altitude to pressure at that altitude, assume pressure alt got calculated with standard pressure at sea level in mind
double PressureFromBaroAlt(double baroAlt_m, double refPressure = HPA_STANDARD);
/// Convert a given pressure to an altitude, providing sea level pressure as reference
double AltFromPressure(double pressure, double refPressure);
/// Convert a barometric altitude (based on std pressure) to a geometric altitude
double BaroAltToGeoAlt_m(double baroAlt_m, double refPressure);
/// Convert a barometric altitude (based on std pressure) to a geometric altitude
inline double BaroAltToGeoAlt_ft(double baroAlt_ft, double refPressure)
{ return BaroAltToGeoAlt_m(baroAlt_ft * M_per_FT, refPressure) / M_per_FT; }

/// Convert a geometric altitude to a barometric altitude (based on std pressure)
double GeoAltToBaroAlt_m(double geoAlt_m, double refPressure);
/// Convert a geometric altitude to a barometric altitude (based on std pressure)
inline double GeoAltToBaroAlt_ft(double geoAlt_ft, double refPressure)
{ return GeoAltToBaroAlt_m(geoAlt_ft * M_per_FT, refPressure) / M_per_FT; }

/// Fetch nearest airport id by location
std::string GetNearestAirportId (const positionTy& _pos, positionTy* outApPos = nullptr);

/// Fetch nearest airport id by location
inline std::string GetNearestAirportId (float lat, float lon)
{ return GetNearestAirportId(positionTy((double)lat,(double)lon)); }

/// Convert ADS-B Emitter Category to text
const char* GetADSBEmitterCat (const std::string& cat);

/// Which plugin has control of AI?
std::string GetAIControlPluginName ();

// convert a color value from int to float[4]
void conv_color ( int inCol, float outCol[4] );

// verifies if one container begins with the same content as the other
// https://stackoverflow.com/questions/931827/stdstring-comparison-check-whether-string-begins-with-another-string
template<class TContainer>
bool begins_with(const TContainer& input, const TContainer& match)
{
    return input.size() >= match.size()
    && std::equal(match.cbegin(), match.cend(), input.cbegin());
}

/// Is value `lo <= v <= hi`?
template<class T>
constexpr bool between( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (lo <= v) && (v <= hi);
}


// comparing 2 doubles for near-equality
bool dequal ( const double d1, const double d2 );

/// Convert NAN to zero, otherwise pass `d`
inline double nanToZero (double d)
{ return std::isnan(d) ? 0.0 : d; }

/// @brief random long between too given values invlusive
/// @see https://stackoverflow.com/a/7560171
inline long randoml (long min, long max)
{ return long(((double) rand() / (RAND_MAX+1.0)) * (max-min+1)) + min; }

// gets latest version info from X-Plane.org
bool FetchXPlaneOrgVersion ();

/// LiveTraffic's version number as pure integer for returning in a dataRef, like 201 for v2.01
int GetLTVerNum(void* = NULL);

/// LiveTraffic's build date as pure integer for returning in a dataRef, like 20200430 for 30-APR-2020
int GetLTVerDate(void* = NULL);

// MARK: Compiler differences

#if APL == 1 || LIN == 1
// not quite the same but close enough for our purposes
inline void strncpy_s(char * dest, size_t destsz, const char * src, size_t count)
{
    strncpy(dest, src, std::min(destsz,count)); dest[destsz - 1] = 0;
}

// these simulate the VC++ version, not the C standard versions!
inline struct tm *gmtime_s(struct tm * result, const time_t * time)
{ return gmtime_r(time, result); }
inline struct tm *localtime_s(struct tm * result, const time_t * time)
{ return localtime_r(time, result); }

#endif

/// Simpler access to strncpy_s if dest is a char array (not a pointer!)
#define STRCPY_S(dest,src) strncpy_s(dest,sizeof(dest),src,sizeof(dest)-1)
#define STRCPY_ATMOST(dest,src) strncpy_s(dest,sizeof(dest),strAtMost(src,sizeof(dest)-1).c_str(),sizeof(dest)-1)

// XCode/Linux don't provide the _s functions, not even with __STDC_WANT_LIB_EXT1__ 1
#if APL
inline int strerror_s( char *buf, size_t bufsz, int errnum )
{ return strerror_r(errnum, buf, bufsz); }
#elif LIN
inline int strerror_s( char *buf, size_t bufsz, int errnum )
{ strerror_r(errnum, buf, bufsz); return 0; }
#endif

// MARK: Thread and Locale

/// Begin a thread and set a thread-local locale
/// @details In the communication with servers we must use internal standards,
///          ie. C locale, so that for example the decimal point is `.`
///          Hence we set a thread-local locale in all threads as they deal with communication.
///          See https://stackoverflow.com/a/17173977
class ThreadSettings {
protected:
#if IBM
#define LC_ALL_MASK LC_ALL
#else
    locale_t threadLocale = locale_t(0);
    locale_t prevLocale = locale_t(0);
#endif
public:
    /// @brief Defines thread's name and sets the thread's locale
    /// @param sThreadName Thread's name, max 16 chars
    /// @param localeMask One of the LC_*_MASK constants. If `0` then locale is not changed.
    /// @param sLocaleName New locale to set
    ThreadSettings (const char* sThreadName,
                    int localeMask = 0,
                    const char* sLocaleName = "C");
    /// Restores and cleans up locale
    ~ThreadSettings();
};

#endif /* LiveTraffic_h */
