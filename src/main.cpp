#include "./main.hpp"
#include "./mainWindow/mainWindow.hpp"

IMPLEMENT_APP(KAnime)

json profile;

bool KAnime::OnInit() { readJson("profile.kanime", profile);
  int mwWidth, mwHeight, mwMinWidth, mwMinHeight;
  try {
    mwWidth = profile["settings"]["screen"]["width"].get<int>(), mwHeight = profile["settings"]["screen"]["height"].get<int>();
    mwMinWidth = profile["settings"]["screen"]["width_min"].get<int>(), mwMinHeight = profile["settings"]["screen"]["height_min"].get<int>();
  } catch (...) { mwWidth = 1045, mwHeight = 550; mwMinWidth = 785, mwMinHeight = 395; }
  profile["settings"]["screen"]["width"] = mwWidth, profile["settings"]["screen"]["height"] = mwHeight;
  profile["settings"]["screen"]["width_min"] = mwMinWidth, profile["settings"]["screen"]["height_min"] = mwMinHeight;


  MainWindow *mainWindow = new MainWindow(wxT("KAnime+"), mwWidth, mwHeight, profile);
  mainWindow->SetMinSize(wxSize(mwMinWidth, mwMinHeight));
  mainWindow->Show(true);

  return true;
}
