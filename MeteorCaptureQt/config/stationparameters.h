#ifndef STATIONPARAMETERS_H
#define STATIONPARAMETERS_H

#include "config/configparameterfamily.h"
#include "config/parameterdouble.h"
#include "config/validate.h"
#include "infra/meteorcapturestate.h"

class StationParameters : public ConfigParameterFamily {

public:

    StationParameters(MeteorCaptureState * state) : ConfigParameterFamily("Station", 3) {

        parameters = new ConfigParameterBase*[numPar];
        validators = new ParameterValidator*[numPar];

        // Create validators for each parameter
        validators[0] = new ValidateWithinLimits<double>(0.0, 360.0);
        validators[1] = new ValidateWithinLimits<double>(-90.0, 90.0);
        validators[2] = new ValidateWithinLimits<double>(-100.0, 5000.0);

        // Create parameters
        parameters[0] = new ParameterDouble("longitude", "Longitude", "deg", validators[0], &(state->longitude));
        parameters[1] = new ParameterDouble("latitude", "Latitude", "deg", validators[1], &(state->latitude));
        parameters[2] = new ParameterDouble("altitude", "Altitude", "m", validators[2], &(state->altitude));
    }

};

#endif // STATIONPARAMETERS_H