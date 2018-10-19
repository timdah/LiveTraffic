//
//  LTAircraft.h
//  LiveTraffic

/*
 * Copyright (c) 2018, Birger Hoppe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef LTAircraft_h
#define LTAircraft_h

#include <string>
#include "XPLMScenery.h"
#include "xplanemp/XPCAircraft.h"
#include "CoordCalc.h"
#include "LTFlightData.h"

//
//MARK: MovingParam
//      Represents a parameter which changes over time, like e.g.
//      "gear", which takes some seconds to go up or down
//
struct MovingParam
{
public:
    // defining parameters
    double defMin, defMax, defDist, defDuration;
    // wrap around at max, i.e. start over at begin?
    // (good for heading, which goes from 0 to 360)
    const bool bWrapAround;
protected:
    // target values (tTime is NaN if we are _not_ moving
    double valFrom, valTo, valDist, timeFrom, timeTo;
    // increase ot decrease values? (really meaningful only if bWrapAround)
    bool bIncrease;
    // actual value
    double val;
    
public:
    // Constructor
    MovingParam(double _dur, double _max=1.0, double _min=0.0,
                bool _wrap_around=false);
    void SetVal (double _val);
    
    // are we in motion? (i.e. moving from val to target?)
    bool inMotion () const;

    // start a move to the given target value
    void moveTo ( double tval );
    inline void up ()       { moveTo(defMin); }
    inline void down ()     { moveTo(defMax); }
    inline void min ()      { moveTo(defMin); }
    inline void max ()      { moveTo(defMax); }

    // pre-program a move, which is to finish by the given time
    void moveToBy (double _from, bool _increase, double _to,
                   double _startTS, double _by_ts,
                   bool _startEarly);
    // pre-program a quick move the shorter way (using wrap around if necessary)
    void moveQuickestToBy (double _from,        // NAN = current val
                           double _to,
                           double _startTS,     // NAN = now
                           double _by_ts,       // when finished with move?
                           bool _startEarly);   // start at _startTS? or finish at _by_ts?

    // get current value (might actually _change_ val if inMotion!)
    double get ();
    
    // non-moving status checks
    double is () const      { return val; }
    bool isUp () const      { return val == defMin; }
    bool isDown () const    { return val == defMax; }
    double toVal () const   { return valTo; }
};

// mimics acceleration / deceleration
struct AccelParam
{
protected:
    double startSpeed, targetSpeed, acceleration, targetDeltaDist;
    double startTime, accelStartTime, targetTime;
    double currSpeed_m_s, currSpeed_kt;      // set during getSpeed
public:
    // default only allows for object init
    AccelParam();
    // Set start/target [m/s], but no acceleration
    void SetSpeed (double speed);
    
    // get current value
    double m_s() const { return currSpeed_m_s; }
    double kt() const { return currSpeed_kt; }
    bool isZero() const { return currSpeed_m_s == 0; }
    
    // start an acceleration now
    void StartAccel(double startSpeed, double targetSpeed, double accel,
                    double startTime=NAN);
    // reach target Speed by targetTime after deltaDist
    void StartSpeedControl(double startSpeed, double targetSpeed,
                           double deltaDist,
                           double startTime, double targetTime);
    
    inline bool isChanging() const { return !isnan(acceleration); }
    
    // calculations (ts = timestamp, defaults to current sim time)
    double updateSpeed ( double ts = NAN );
    double getDeltaDist ( double ts = NAN ) const;
    double getRatio ( double ts = NAN ) const;
    inline double getTargetTime() const         { return targetTime; }
    inline double getTargetDeltaDist() const    { return targetDeltaDist; }
};

//
//MARK: LTAircraft
//      Represents an aircrafts as displayed in XP by use of the
//      XP Multiplayer Lib
//
class LTAircraft : XPCAircraft
{
public:
    class FlightModel {
    public:
        std::string modelName;
        double GEAR_DURATION =    10;     // time for gear up/down
        double FLAPS_DURATION =   5;      // time for full flaps extension from 0% to 100%
        double VSI_STABLE =       100;    // [ft/min] less than this VSI is considered 'stable'
        double ROTATE_TIME =      3;      // [s] to rotate before lift off
        double VSI_FINAL =        -600;   // [ft/min] assumed vsi for final if vector unavailable
        double VSI_INIT_CLIMB =   1500;   // [ft/min] assumed vsi if take-off-vector not available
        double SPEED_INIT_CLIMB = 150;    // [kt] initial climb speed if take-off-vector not available
        double AGL_GEAR_DOWN =    1600;   // height AGL at which to lower the gear during approach
        double AGL_GEAR_UP =      100;    // height AGL at which to raise the gear during take off
        double AGL_FLARE =        25;     // [ft] height AGL to start flare in artifical pos mode
        double MAX_TAXI_SPEED =   50;     // below that: taxi, above that: take-off/roll-out
        double TAXI_TURN_TIME =   45;     // seconds for a 360° turn on the ground
        double FLAPS_UP_SPEED =  180;     // below that: initial climb, above that: climb
        double FLAPS_DOWN_SPEED =  200;   // above that: descend, below that: approach
        double CRUISE_HEIGHT =    15000;  // above that height AGL we consider level flight 'cruise'
        double ROLL_OUT_DECEL =  -2.0;    // [m/s²] deceleration during roll-out
        double PITCH_MIN =        -2;     // [°] minimal pitch angle (aoa)
        double PITCH_MIN_VSI =    -1000;  // [ft/min] minimal vsi below which pitch is MDL_PITCH_MIN
        double PITCH_MAX =        18;     // [°] maximum pitch angle (aoa)
        double PITCH_MAX_VSI =    2000;   // [ft/min] maximum vsi above which pitch is MDL_PITCH_MAX
        double PITCH_FLAP_ADD =   4;      // [°] to add if flaps extended
        double PITCH_FLARE =      10;     // [°] pitch during flare
        double PITCH_RATE =       5;      // [°/s] pitch rate of change
        
        // modelName is key, so base comparison on it
    public:
        bool operator == (const FlightModel& o) const { return modelName == o.modelName; }
        bool operator <  (const FlightModel& o) const { return modelName <  o.modelName; }
        bool operator >  (const FlightModel& o) const { return modelName >  o.modelName; }
        operator bool () const { return !modelName.empty(); }

    public:
        static bool ReadFlightModelFile ();
        static const FlightModel& FindFlightModel (const std::string acTypeIcao);
    };
    
public:
    enum FlightPhase {
        FPH_UNKNOWN     = 0,
        FPH_TAXI        = 10,
        FPH_TAKE_OFF    = 20,
        FPH_TO_ROLL,
        FPH_ROTATE,
        FPH_LIFT_OFF,
        FPH_INITIAL_CLIMB,
        FPH_CLIMB       = 30,
        FPH_CRUISE      = 40,
        FPH_DESCEND     = 50,
        FPH_APPROACH    = 60,
        FPH_FINAL,
        FPH_LANDING     = 70,
        FPH_FLARE,
        FPH_TOUCH_DOWN,                 // this is a one-frame-only 'phase'!
        FPH_ROLL_OUT,
        FPH_STOPPED_ON_RWY              // ...after artifically roll-out with no more live positions remaining
    };
    static std::string FlightPhase2String (FlightPhase phase);
    
public:
    // reference to the defining flight data
    LTFlightData& fd;
    // reference to the flight model being used
    const FlightModel& mdl;
    
    // absolute positions (max 3: last, current destination, next)
    // as basis for calculating ppos per frame
    dequePositionTy      posList;
    
    XPMPPlaneSurfaces_t surfaces;
    XPMPPlaneRadar_t    radar;
    std::string         labelAc;        // label at the a/c
    std::string         labelInternal;  // internal label, e.g. for error messages
protected:
    // this is "ppos", the present simulated position,
    // where the aircraft is to be drawn
    positionTy          ppos;
    // and this the current vector from 'from' to 'to'
    vectorTy            vec;
    
    // timestamp we last requested new positions from flight data
    double              tsLastCalcRequested;
    
    // dynamic parameters of the plane
    FlightPhase         phase;          // current flight phase
    double              rotateTs;       // when to rotate?
    double              vsi;            // vertical speed (ft/m)
    bool                bOnGrnd;        // are we touching ground?
    bool                bArtificalPos;  // running on artifical positions for roll-out?
    bool                bNeedNextVec;   // in need of next vector after to-pos?
    AccelParam          speed;          // current speed [m/s] and acceleration control
    MovingParam         gear;
    MovingParam         flaps;
    MovingParam         heading;        // used when turning
    MovingParam         pitch;
    
    // Y-Probe
    XPLMProbeRef        probeRef;
    double              probeNextTs;    // timestamp of NEXT probe
    double              terrainAlt;     // in feet
    
    // bearing/dist from viewpoint to a/c
    vectorTy            vecView;        // degrees/meters
    
    // object valid? (set to false after exceptions)
    bool                bValid;
public:
    LTAircraft(LTFlightData& fd);
    virtual ~LTAircraft();
    
    // key for maps
    inline const std::string& key() const { return fd.key(); }
    // labels to pin to aircrafts on the screes
    inline const std::string& label() const { return labelAc; }
    // stringify e.g. for debugging info purposes
    operator std::string() const;
    // the XPMP model used for displaying this aircraft
    std::string GetModelName() const;
    // current position
    inline const positionTy& GetPPos() const { return ppos; }
    inline positionTy GetPPosLocal() const { return positionTy(ppos).WorldToLocal(); }
    // position heading to (usually posList[1], ppos if ppos > posList[1])
    const positionTy& GetToPos() const;
    // have no more viable positions left, in need of more?
    bool OutOfPositions() const;
    // current a/c configuration
    inline FlightPhase GetFlightPhase() const { return phase; }
    inline bool IsOnGrnd() const { return bOnGrnd; }
    inline double GetFlapsPos() const { return flaps.is(); }
    inline double GetGearPos() const { return gear.is(); }
    inline double GetSpeed_kt() const { return speed.kt(); }                     // kt
    inline double GetSpeed_m_s() const { return speed.m_s(); }   // m/s
    inline double GetVSI_ft() const { return vsi; }                         // ft/m
    inline double GetVSI_m_s() const { return vsi * Ms_per_FTm; }           // m/s
    inline double GetPitch() const { return ppos.pitch(); }
    inline double GetRoll() const { return ppos.roll(); }
    inline double GetTerrainAlt_ft() const { return terrainAlt; }           // ft
    inline double GetTerrainAlt_m() const { return terrainAlt * M_per_FT; } // m
    inline double GetPHeight_ft() const { return ppos.alt_ft() - terrainAlt; }
    inline double GetPHeight_m() const { return GetPHeight_ft() * M_per_FT; }
    inline vectorTy GetVecView() const { return vecView; }
    // object valid? (set to false after exceptions)
    inline bool IsValid() const { return bValid; }

protected:
    // based on current sim time and posList calculate the present position
    bool CalcPPos ();
    // determine other parameters like gear, flap, roll etc. based on flight model assumptions
    void CalcFlightModel (const positionTy& from, const positionTy& to);
    bool YProbe ();

    // XPMP Aircraft Updates (callbacks)
    virtual XPMPPlaneCallbackResult GetPlanePosition(XPMPPlanePosition_t* outPosition);
    virtual XPMPPlaneCallbackResult GetPlaneSurfaces(XPMPPlaneSurfaces_t* outSurfaces);
    virtual XPMPPlaneCallbackResult GetPlaneRadar(XPMPPlaneRadar_t* outRadar);
};

//MARK: Control functions
// ( probably to be moved to a separate header )

bool LTMainInit ();
bool LTMainEnable ();
bool LTMainShowAircraft ();
void LTMainHideAircraft ();
void LTMainDisable ();
void LTMainStop ();

#endif /* LTAircraft_h */
