#include <trview.app/Application.h>
#include <trview.app/Mocks/Menus/IUpdateChecker.h>
#include <trview.app/Mocks/Settings/ISettingsLoader.h>
#include <trview.app/Mocks/Menus/IFileDropper.h>
#include <trlevel/Mocks/ILevelLoader.h>

using namespace trview;
using namespace trview::tests;
using namespace testing;

TEST(Application, ChecksForUpdates)
{
    auto [update_checker_ptr, update_checker] = create_mock<mocks::MockUpdateChecker>();
    EXPECT_CALL(update_checker, check_for_updates).Times(1);
    CoInitialize(nullptr);
    Application application(create_test_window(L"ApplicationTests"),
        std::move(update_checker_ptr),
        std::make_unique<mocks::MockSettingsLoader>(),
        std::make_unique<mocks::MockFileDropper>(),
        std::make_unique<trlevel::mocks::MockLevelLoader>(),
        std::wstring());
}

TEST(Application, SettingsLoadedAndSaved)
{
    auto [settings_loader_ptr, settings_loader] = create_mock<mocks::MockSettingsLoader>();
    EXPECT_CALL(settings_loader, load_user_settings).Times(1);
    EXPECT_CALL(settings_loader, save_user_settings).Times(1);
    CoInitialize(nullptr);
    Application application(create_test_window(L"ApplicationTests"),
        std::make_unique<mocks::MockUpdateChecker>(),
        std::move(settings_loader_ptr),
        std::make_unique<mocks::MockFileDropper>(),
        std::make_unique<trlevel::mocks::MockLevelLoader>(),
        std::wstring());
}

TEST(Application, FileDropperOpensFile)
{
    auto [file_dropper_ptr, file_dropper] = create_mock<mocks::MockFileDropper>();
    auto [level_loader_ptr, level_loader] = create_mock<trlevel::mocks::MockLevelLoader>();
    CoInitialize(nullptr);

    EXPECT_CALL(level_loader, load_level("test_path.tr2"))
        .Times(1)
        .WillRepeatedly(Throw(std::exception()));

    Application application(create_test_window(L"ApplicationTests"),
        std::make_unique<mocks::MockUpdateChecker>(),
        std::make_unique<mocks::MockSettingsLoader>(),
        std::move(file_dropper_ptr),
        std::move(level_loader_ptr),
        std::wstring());
    file_dropper.on_file_dropped("test_path.tr2");
}


