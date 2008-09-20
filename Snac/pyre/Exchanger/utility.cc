// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "journal/error.h"
#include "utility.h"


MPI_Status util::waitRequest(const MPI_Request& request)
{
    MPI_Status status;
    int result = MPI_Wait(const_cast<MPI_Request*>(&request), &status);
    testResult(result, "wait error!");

    return status;
}


std::vector<MPI_Status>
util::waitRequest(const std::vector<MPI_Request>& request)
{
    std::vector<MPI_Status> status(request.size());
    int result = MPI_Waitall(request.size(),
			     const_cast<MPI_Request*>(&request[0]), &status[0]);
    testResult(result, "wait error!");

    return status;
}


void util::testResult(int result, const std::string& errmsg)
{
    if (result != MPI_SUCCESS) {
        journal::error_t error("utility");
        error << journal::at(__HERE__)
              << errmsg << journal::endl;
	throw result;
    }
}





// version
// $Id: utility.cc 662 2004-01-20 12:34:17Z PatrickSunter $

// End of file
