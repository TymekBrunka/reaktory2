#include "Translations.hpp"
#include <string_view>
namespace Log {

std::string_view messages_pl[MSG_S_COUNT] = {
    [MSG_RAW] = "{}",

    [MSG_APP_HOME_DIR_ERROR] = "Nie można znaleźć folderu użytkownika.",
    [MSG_APP_ROOT_CREATE_ERROR] = "Nie można utworzyć folderu aplikacji.",
    [MSG_APP_ROOT_SUBDIR_CREATE_ERROR] = "Nie można utworzyć folderu '{}'.",

    [MSG_RENDER_INIT_ERROR] = "Nie można zainicjować renderowania.",
    [MSG_RENDER_INIT_SUCCESS] = "Zainicjowano renderer",
    [MSG_RENDER_CREATE_WINDOW_SUCCESS] = "Utworzono nowe okno",
    [MSG_RENDER_CREATE_WINDOW_FAILURE] = "Nie można utworzyć nowego okna.",
    [MSG_RENDER_SET_WINDOW_CALLBACKS] = "Ustawiono zdarzenia okna",
    [MSG_RENDER_SET_GLOBAL_GL_CTX] =
        "Ustawiono globalny kontekst opengl (dla imgui)",
    [MSG_RENDER_CREATE_IMGUI_CTX] = "Utworzono kontekst imgui",

    [MSG_GL_ERROR_SHADER] = "Nie można skompilować {} shader'a: {}.",
    [MSG_GL_ERROR_PROGRAM] =
        "Nie można podlinkować programu opengl \"{}\" : {}.",
    [MSG_GL_ERROR_VALIDATION] =
        "Wystąpił błąd podczas walidacji programu opengl: {}.",
};

std::string_view messages_en[MSG_S_COUNT] = {
    [MSG_RAW] = "{}",

    [MSG_APP_HOME_DIR_ERROR] = "Cannot find home directory.",
    [MSG_APP_ROOT_CREATE_ERROR] = "Cannot create app root directory.",
    [MSG_APP_ROOT_SUBDIR_CREATE_ERROR] = "Cannot create directory '{}'.",

    [MSG_RENDER_INIT_ERROR] = "Couldn't initialise renderer.",
    [MSG_RENDER_INIT_SUCCESS] = "Initialised renderer",
    [MSG_RENDER_CREATE_WINDOW_SUCCESS] = "Created new window",
    [MSG_RENDER_CREATE_WINDOW_FAILURE] = "Couldn't create new window.",
    [MSG_RENDER_SET_WINDOW_CALLBACKS] = "Set window callbacks",
    [MSG_RENDER_SET_GLOBAL_GL_CTX] = "Set global opengl context (for imgui)",
    [MSG_RENDER_CREATE_IMGUI_CTX] = "Created imgui context",

    [MSG_GL_ERROR_SHADER] = "Cannot compile {} shader: {}.",
    [MSG_GL_ERROR_PROGRAM] = "Cannot link opengl program \"{}\" : {}.",
    [MSG_GL_ERROR_VALIDATION] = "Opengl program validation returned error: {}.",
};

} // namespace Log
