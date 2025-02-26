// SPDX-License-Identifier: Apache-2.0

#include "config.h"

#include "hardware_isolation_manager.hpp"
#include "utils.hpp"

#include <fmt/format.h>

#include <phosphor-logging/elog-errors.hpp>
#include <sdeventplus/event.hpp>

int main()
{
    auto eventLoopRet = 0;
    try
    {
        hw_isolation::utils::initExternalModules();

        auto bus = sdbusplus::bus::new_default();
        bus.request_name(HW_ISOLATION_BUSNAME);

        auto event = sdeventplus::Event::get_default();
        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);

        // Add sdbusplus ObjectManager for the 'root' path of the hardware
        // isolation manager.
        sdbusplus::server::manager::manager objManager(bus,
                                                       HW_ISOLATION_OBJPATH);

        hw_isolation::Manager hardware_isolation_mgr(bus, HW_ISOLATION_OBJPATH,
                                                     event.get());

        // Restore the isolated hardwares from their persisted location.
        hardware_isolation_mgr.restore();

        // The below statement should be last to enter this app into the loop
        // to process D-Bus services.
        eventLoopRet = event.loop();
    }
    catch (std::exception& e)
    {
        using namespace phosphor::logging;
        log<level::ERR>(fmt::format("Exception [{}]", e.what()).c_str());
    }

    return eventLoopRet;
}
