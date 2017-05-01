#include "stdafx.h"
#include "UserInterface/PatreonEmail.h"

#include <regex>

bool ValidPatreonEmail (void)
{
    std::string email = UISettingsLoadStringVal(SupportWindows_PatreonEmail);
    const std::regex pattern("(\\w+)@(\\w+)(\\.(\\w+))+");
    bool valid = std::regex_match(email, pattern);
    return valid;
}

void OpenPatreonPage(void)
{
    ShellExecute(NULL, "open", "https://www.patreon.com/bePatron?c=796868", NULL, NULL, SW_SHOWMAXIMIZED);
}

void ChangePatreonEmail(HWND hParent)
{
    CPatreonEmail PatreonEmailWindow;
    PatreonEmailWindow.DoModal(hParent);
}
