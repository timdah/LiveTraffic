/// @file       LTFlightRadar.cpp
/// @brief      FlightRadar24: Requests and processes live tracking data
/// @see        https://www.flightradar24.com/
/// @author     Tim Dahlmanns
/// @copyright  (c) 2024-2024 Tim Dahlmanns
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

// All includes are collected in one header
#include "LTFlightRadar.h"
#include "Constants.h"
#include "CoordCalc.h"
#include "LTChannel.h"
#include "parson.h"
#include <cmath>
#include <curl/curl.h>
#include <string>
#include <vector>

// Constructor
FlightRadarConnection::FlightRadarConnection () :
LTFlightDataChannel(DR_CHANNEL_FLIGHT_RADAR_ONLINE, FR_NAME) 
{
    // purely informational
    urlName  = FR_CHECK_NAME;
    urlLink  = FR_CHECK_URL;
    urlPopup = FR_CHECK_POPUP;
}

// virtual thread main function
void FlightRadarConnection::Main ()
{
    // This is a communication thread's main function, set thread's name and C locale
    ThreadSettings TS ("LT_FR", LC_ALL_MASK);
    
    while ( shallRun() ) {
        // LiveTraffic Top Level Exception Handling
        try {
            // basis for determining when to be called next
            tNextWakeup = std::chrono::steady_clock::now();
            
            // where are we right now?
            const positionTy pos (dataRefs.GetViewPos());
            
            // If the camera position is valid we can request data around it
            if (pos.isNormal()) {
                // Next wakeup is "refresh interval" from _now_
                tNextWakeup += std::chrono::seconds(dataRefs.GetFdRefreshIntvl());
                
                // fetch data and process it
                if (FetchAllData(pos) && ProcessFetchedData())
                        // reduce error count if processed successfully
                        // as a chance to appear OK in the long run
                        DecErrCnt();
            }
            else {
                // Camera position is yet invalid, retry in a second
                tNextWakeup += std::chrono::seconds(1);
            }
            
            // sleep for FD_REFRESH_INTVL or if woken up for termination
            // by condition variable trigger
            {
                std::unique_lock<std::mutex> lk(FDThreadSynchMutex);
                FDThreadSynchCV.wait_until(lk, tNextWakeup,
                                           [this]{return !shallRun();});
            }
            
        } catch (const std::exception& e) {
            LOG_MSG(logERR, ERR_TOP_LEVEL_EXCEPTION, e.what());
            IncErrCnt();
        } catch (...) {
            LOG_MSG(logERR, ERR_TOP_LEVEL_EXCEPTION, "(unknown type)");
            IncErrCnt();
        }
    }
}

// put together the URL to fetch based on current view position
std::string FlightRadarConnection::GetURL (const positionTy& pos)
{
    // we add 10% to the bounding box to have some data ready once the plane is close enough for display
    boundingBoxTy box (pos, double(dataRefs.GetFdStdDistance_m()) * 1.10);
    char url[128] = "";
    snprintf(url, sizeof(url),
             FR_URL,
             box.nw.lat(),              // lamax
             box.se.lat(),              // lamin
             box.nw.lon(),              // lomin
             box.se.lon() );            // lomax
    std::string result = std::string(url);
    return result;
}

bool FlightRadarConnection::ProcessFetchedData()
{
    // data is expected to be in netData string
    // short-cut if there is nothing
    if (!netDataPos) {
        IncErrCnt();
        return false;
    }
    
    // now try to interpret it as JSON
    JSONRootPtr pRoot(netData);
    if (!pRoot) { LOG_MSG(logERR,ERR_JSON_PARSE); IncErrCnt(); return false; }
    
    // first get the structure's main object
    JSON_Object* pObj = json_object(pRoot.get());
    if (!pObj) { LOG_MSG(logERR,ERR_JSON_MAIN_OBJECT); IncErrCnt(); return false; }
    
    // We need to calculate distance to current camera later on
    const positionTy viewPos = dataRefs.GetViewPos();
    
    // Get the current timestamp
    double now = dataRefs.GetSimTime();

    // Remove the full_count and version fields
    json_object_remove(pObj, "full_count");
    json_object_remove(pObj, "version");

    // Iterate over each aircraft in the JSON object
    for (size_t i=0; i < json_object_get_count(pObj); i++) 
    {
        const char* flightId = json_object_get_name(pObj, i);
        // Get the aircraft data array
        JSON_Array* pJAc = json_object_get_array(pObj, flightId);
        if (!pJAc || json_array_get_count(pJAc) < 19) {
            LOG_MSG(logERR, ERR_JSON_AC, flightId);
            IncErrCnt();
            continue;
        }

        try {
            // Extract the relevant fields
            std::string icao      = jag_s(pJAc, FR_TRANSP_ICAO);
            std::string feeder    = jag_s(pJAc, FR_FEEDER);
            std::string acType    = jag_s(pJAc, FR_AC_TYPE);
            std::string reg       = jag_s(pJAc, FR_REGISTRATION);
            std::string origin    = jag_s(pJAc, FR_ORIGIN);
            std::string dest      = jag_s(pJAc, FR_DESTINATION);
            std::string flNr      = jag_s(pJAc, FR_FLIGHT_NR);
            std::string callSgn   = jag_s(pJAc, FR_CALL);
            std::string airline   = jag_s(pJAc, FR_AIRLINE);
            double lat            = jag_n_nan(pJAc, FR_LAT);
            double lon            = jag_n_nan(pJAc, FR_LON);
            double track          = jag_n_nan(pJAc, FR_HEADING);
            double baroAlt_ft     = jag_n_nan(pJAc, FR_CALC_ALT);
            double speed          = jag_n_nan(pJAc, FR_SPD);
            double vertSpeed      = jag_n_nan(pJAc, FR_VERT_SPD);
            double posTime        = jag_n_nan(pJAc, FR_POS_TIME);

            // Discard incomplete core AC data
            if (
                    icao.empty() ||
                    isnan(lat) ||
                    isnan(lon) ||
                    isnan(track) ||
                    isnan(baroAlt_ft) ||
                    isnan(posTime) ||
                    isnan(speed)
                )
                continue;

            // Discard data older than simulation time
            if (posTime <= now)
                continue;

            // Create the fdKey
            LTFlightData::FDKeyTy fdKey(LTFlightData::KEY_ICAO, icao);

            // AC on ground?
            bool onGround = baroAlt_ft <= 20; // random threshold. fr24 sets alt to 0 on ground

            // Position information
            const double geoAlt_ft = BaroAltToGeoAlt_ft(baroAlt_ft, dataRefs.GetPressureHPA());
            positionTy acPos(lat, lon, geoAlt_ft * M_per_FT, posTime, track);
            acPos.f.onGrnd = onGround ? GND_ON : GND_OFF;
            acPos.heading() = track;

            // Calculate the distance to the camera
            double dist = acPos.dist(viewPos);

            // Access fdMap guarded by a mutex
            std::unique_lock<std::mutex> mapFdLock (mapFdMutex);

            // Get or create the LTFlightData object
            LTFlightData& fd = mapFd[fdKey];

            // Get the data access lock 
            std::lock_guard<std::recursive_mutex> fdLock (fd.dataAccessMutex);
            mapFdLock.unlock();

            // Fill key fields if new
            if (fd.empty())
                fd.SetKey(fdKey);

            // Fill static data
            LTFlightData::FDStaticData stat;
            stat.acTypeIcao = acType;
            stat.call = callSgn;
            stat.reg = reg;
            stat.stops = {origin, dest};
            stat.flight = flNr;
            stat.opIcao = airline;

            // Dynamic data
            LTFlightData::FDDynamicData dyn;
            dyn.gnd = onGround;
            dyn.heading = track;
            dyn.spd = speed;
            dyn.vsi = vertSpeed;
            dyn.ts = now;
            dyn.pChannel = this;

            // Update data
            fd.UpdateData(std::move(stat), dist);

            // Add dynamic data if position is valid
            if (acPos.isNormal(false)) {
                fd.AddDynData(dyn, 0, 0, &acPos);
            }
            else {
                LOG_MSG(logDEBUG,ERR_POS_UNNORMAL,fdKey.c_str(),acPos.dbgTxt().c_str());
            }
        } catch(const std::system_error& e) {
            LOG_MSG(logERR, ERR_LOCK_ERROR, "mapFd", e.what());
        }
        
    }
    
    // success
    return true;
}
  
