// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef O2_FRAMEWORK_TERMINATIONPOLICY_H_
#define O2_FRAMEWORK_TERMINATIONPOLICY_H_

namespace o2::framework
{

/// These are the possible actions we can do
/// when a workflow is deemed complete (e.g. when we are done
/// reading from file) or when a data processor has
/// an error.
enum struct TerminationPolicy { QUIT,
                                WAIT,
                                RESTART };

} // namespace o2::framework

#endif // O2_FRAMEWORK_TERMINATIONPOLICY_H_