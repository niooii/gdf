#pragma once

#undef min
#undef max
#ifndef GDF_RELEASE
#define HFSM2_ENABLE_LOG_INTERFACE
#define HFSM2_ENABLE_VERBOSE_DEBUG_LOG
#endif
#include <hfsm2/machine.hpp>

// helper macro for forward declaring structs
#define S(s) struct s

#ifndef GDF_RELEASE
// useless ass logger
struct HfsmLogger
    : hfsm2::LoggerInterface
{
    // void recordMethod(const Context& /*context*/,
    //                   const hfsm2::StateID /*origin*/,
    //                   const Method method)								override
    // {
    //     LOG_TRACE("%s::%s", hfsm2::stateName(origin));
    //     std::cout //<< hfsm2::stateName(origin) << "::"
    //               << hfsm2::methodName(method) << "()\n";
    // }

    // void recordTransition(
    //     const Context& context,
    //     const hfsm2::StateID origin,
    //     const TransitionType transitionType,
    //     const hfsm2::StateID target,
    //     const ) override
    // {
    //     // LOG_TRACE("%d %s %d", hfsm2::stateName(origin), hfsm2::transitionName(transitionType), target);
    // }
};

static HfsmLogger HFSM_DEBUG_LOGGER{};
#endif
