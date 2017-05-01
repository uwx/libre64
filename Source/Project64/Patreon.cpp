#include "stdafx.h"
#include "UserInterface/PatreonEmail.h"

bool ValidPatreonEmail (void)
{
    std::string email = UISettingsLoadStringVal(SupportWindows_PatreonEmail);
	std::string::iterator b=email.begin(), e=email.end();

	bool valid = (b=std::find(b, e, '@')) != e && std::find(b, e, '.') != e;
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
