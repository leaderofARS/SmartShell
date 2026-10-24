// SmartShell -- Application Entry Point (Phase 1 Baseline & Foundation Demonstration)
#include "../core/foundation/version.hpp"
#include "../core/logging/logger.hpp"
#include "../core/logging/sinks/console_sink.hpp"
#include "../core/logging/sinks/debug_sink.hpp"
#include "../core/configuration/store.hpp"
#include "../core/diagnostics/context.hpp"
#include "../platform/windows/info/os_info.hpp"
#include "../platform/windows/filesystem/filesystem.hpp"
#include "../platform/windows/environment/environment.hpp"
#include "../platform/windows/process/process.hpp"

#include <iostream>
#include <string_view>
#include <memory>

int wmain(int argc, wchar_t* argv[]) {
    // -----------------------------------------------------------------------
    // CLI Arguments (e.g. --version)
    // -----------------------------------------------------------------------
    if (argc > 1 && std::wstring_view{argv[1]} == L"--version") {
        std::cout << "SmartShell " << SMARTSHELL_VERSION << '\n';
        return 0;
    }

    std::cout << "==================================================\n";
    std::cout << " SmartShell " << SMARTSHELL_VERSION << " (Core Platform Phase 1)\n";
    std::cout << " Native Windows Terminal & Shell Platform\n";
    std::cout << "==================================================\n";

    // -----------------------------------------------------------------------
    // 1. Initialize Logging Subsystem
    // -----------------------------------------------------------------------
    ss::Logger logger;
    logger.addSink(std::make_unique<ss::ConsoleSink>(true));
    logger.addSink(std::make_unique<ss::DebugSink>());
    logger.setMinLevel(ss::LogLevel::Trace);

    logger.info(ss::LogCategory::Application, "SmartShell initializing...");

    // -----------------------------------------------------------------------
    // 2. Initialize Diagnostics & Report Platform State
    // -----------------------------------------------------------------------
    ss::DiagnosticContext diagContext{
        .operation = "Startup",
        .component = "ApplicationRoot",
        .resourceId = "SmartShell.exe",
        .sessionId = "",
        .threadId = 0,
        .timestamp = ss::WallClock::now(),
        .nativeError = std::nullopt,
    };
    logger.debug(ss::LogCategory::Diagnostics, diagContext.toString());

    auto osInfo = ss::platform::windows::getOsInfo();
    logger.info(ss::LogCategory::Platform, "Host OS: " + osInfo.toString());
    logger.info(ss::LogCategory::Platform,
                std::string{"Process elevated: "} + (ss::platform::windows::isElevated() ? "yes" : "no"));

    // -----------------------------------------------------------------------
    // 3. Initialize Configuration Primitives & Load Baseline
    // -----------------------------------------------------------------------
    ss::ConfigStore config;
    config.set(ss::ConfigLayer::Defaults, "shell.prompt.style", ss::ConfigValue{ss::Utf8String{"default"}});
    config.set(ss::ConfigLayer::Defaults, "terminal.font.size", ss::ConfigValue{static_cast<ss::i64>(14)});
    config.set(ss::ConfigLayer::User,     "terminal.font.size", ss::ConfigValue{static_cast<ss::i64>(16)});

    auto fontSize = config.getInteger("terminal.font.size");
    if (fontSize) {
        logger.info(ss::LogCategory::Configuration,
                    "Loaded effective configuration: terminal.font.size = " + std::to_string(*fontSize));
    }

    // -----------------------------------------------------------------------
    // 4. Temporary Filesystem Workspace
    // -----------------------------------------------------------------------
    auto tempDirResult = ss::platform::windows::fs::createTempDirectory();
    if (tempDirResult.isError()) {
        logger.fatal(ss::LogCategory::Filesystem, "Failed to create temporary workspace: " + tempDirResult.error().toString());
        return 1;
    }
    auto tempDir = std::move(tempDirResult.value());
    logger.info(ss::LogCategory::Filesystem, "Temporary workspace created: " + tempDir.toUtf8Safe());

    auto demoFile = tempDir / ss::platform::windows::WindowsPath{std::filesystem::path{L"bootstrap.txt"}};
    auto writeStatus = ss::platform::windows::fs::writeTextFile(demoFile, "SmartShell Phase 1 verified.\n");
    if (writeStatus.isError()) {
        logger.error(ss::LogCategory::Filesystem, "Failed to write workspace file: " + writeStatus.error().toString());
    } else {
        logger.debug(ss::LogCategory::Filesystem, "Workspace file written and verified successfully.");
    }

    // -----------------------------------------------------------------------
    // 5. Environment Inspection
    // -----------------------------------------------------------------------
    auto curDir = ss::platform::windows::Environment::getCurrentDirectory();
    if (curDir.isOk()) {
        logger.info(ss::LogCategory::Process, "Working directory: " + curDir.value());
    }

    // -----------------------------------------------------------------------
    // 6. Controlled Child Process Execution (Validation of process foundation)
    // -----------------------------------------------------------------------
    logger.info(ss::LogCategory::Process, "Launching controlled validation process (cmd.exe)...");
    auto procResult = ss::platform::windows::ProcessBuilder{}
        .executable(ss::platform::windows::WindowsPath{std::filesystem::path{L"C:\\Windows\\System32\\cmd.exe"}})
        .arguments({"/c", "exit 0"})
        .build();

    if (procResult.isError()) {
        logger.error(ss::LogCategory::Process, "Failed to launch process: " + procResult.error().toString());
    } else {
        auto& proc = procResult.value();
        logger.debug(ss::LogCategory::Process, "Process started with PID: " + proc.processId().toString());

        auto exitResult = proc.waitForExit(ss::Duration{std::chrono::seconds(5)});
        if (exitResult.isOk()) {
            logger.info(ss::LogCategory::Process,
                        "Process exited cleanly with exit code: " + std::to_string(exitResult.value()));
        } else {
            logger.error(ss::LogCategory::Process, "Process wait failed: " + exitResult.error().toString());
        }
    }

    // -----------------------------------------------------------------------
    // 7. Deterministic Teardown & Resource Cleanup
    // -----------------------------------------------------------------------
    logger.info(ss::LogCategory::Application, "Cleaning up resources...");
    auto delStatus = ss::platform::windows::fs::deleteDirectory(tempDir, true);
    if (delStatus.isError()) {
        logger.warning(ss::LogCategory::Filesystem, "Failed to delete temporary workspace: " + delStatus.error().toString());
    } else {
        logger.debug(ss::LogCategory::Filesystem, "Temporary workspace cleaned up.");
    }

    logger.info(ss::LogCategory::Application, "SmartShell Phase 1 startup & demonstration complete.");
    logger.flush();

    return 0;
}
