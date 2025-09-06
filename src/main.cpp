#include <ctime>

#include <paf.h>
#include <paf/std/string.h>
#include <paf/widget/core/event.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>

#include <curlpp/cURLpp.hpp>
#include <rapidxml.hpp>

using namespace rapidxml;

paf::Framework *g_fw;
paf::Plugin *g_samplePlugin;
paf::ui::Scene *g_rootPage;

paf::ui::Plane *g_pPlane;
paf::ui::Plane *g_pPlaneMain;
paf::ui::Scene *g_pScene;
paf::ui::CornerButton *g_pReturnCornerButton;
paf::ui::CornerButton *g_pSettingsCornerButton;
paf::ui::Plane *g_pPlaneRoot;

class State {
public:
  virtual void enter();
  virtual ~State() = default;
};

class MenuItemFactory : public paf::ui::listview::ItemFactory {
private:
  uint16_t index;

public:
  std::vector<std::pair<paf::wstring, std::optional<paf::ui::HandlerCB>>>
      entries;
  MenuItemFactory(
      std::vector<std::pair<paf::wstring, std::optional<paf::ui::HandlerCB>>>
          map)
      : entries{map} {
    index = 0;
  };

  void Start(StartParam &param) {
    param.list_item->Show(paf::common::transition::Type_FadeinSlow);
  }

  void Stop(StopParam &param) {
    param.list_item->Hide(paf::common::transition::Type_FadeinSlow);
  }

  paf::ui::ListItem *Create(CreateParam &param) {

    paf::Plugin::TemplateOpenParam openParam;
    int res = g_samplePlugin->TemplateOpen(
        param.parent, "_sample_template_list_item", openParam);
    if (res != 0) {
      sceClibPrintf("TemplateOpen 0x%X\n", res);
    }

    paf::ui::ListItem *list_item = (paf::ui::ListItem *)param.parent->GetChild(
        param.parent->GetChildrenNum() - 1);
    paf::ui::Widget *button = list_item->FindChild("button");

    auto entry = entries[index];
    index += 1;
    button->SetString(entry.first.c_str());
    if (!entry.second.has_value()) {
      button->SetShowAlpha(0.3);
    } else {
      button->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                               entry.second.value());
    }
    return list_item;
  }
};

class MainMenu : public State {
public:
  virtual void enter() {
    paf::common::transition::Do(
        0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack1,
        false, false);

    paf::ui::Plane *pPlaneSub =
        (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
    pPlaneSub->Show(NULL, 2);
    paf::common::transition::Do(
        0.0f, pPlaneSub, paf::common::transition::Type::Type_3D_SlideFromFront,
        false, false);

    g_pReturnCornerButton->Show(NULL, 2);
    paf::common::transition::Do(0.0f, g_pReturnCornerButton,
                                paf::common::transition::Type::Type_Popup5,
                                false, false);
    g_pSettingsCornerButton->Show(NULL, 2);
    paf::common::transition::Do(0.0f, g_pSettingsCornerButton,
                                paf::common::transition::Type::Type_Popup5,
                                false, false);
  }
  virtual ~MainMenu();
};

class ArticleList : public State {
public:
  virtual void enter();
  virtual ~ArticleList();
};

class ArticleView : public State {
public:
  virtual void enter();
  virtual ~ArticleView();
};

class ArticleWebview : public ArticleView {
public:
  virtual void enter();
  virtual ~ArticleWebview();
};

class SettingsView : public State {
public:
  virtual void enter();
  virtual ~SettingsView();
};

State *g_currentState;

/*
{

  {
    paf::common::transition::DoReverse(
        0.0f, g_pReturnCornerButton,
        paf::common::transition::Type::Type_Popup5, false, false);
    paf::common::transition::DoReverse(
        0.0f, g_pSettingsCornerButton,
        paf::common::transition::Type::Type_Popup5, false, false);
    paf::Timer *pTimer =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    g_pReturnCornerButton->Hide(pTimer, 2);
    paf::Timer *pTimer2 =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    g_pSettingsCornerButton->Hide(pTimer2, 2);
  }

  {
    paf::ui::Plane *pPlaneSub =
        (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
    paf::Timer *pTimer =
        new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
    pPlaneSub->Hide(pTimer, 2);
    paf::common::transition::DoReverse(
        0.0f, pPlaneSub, paf::common::transition::Type::Type_FadeinFast,
        false, false);
  }

  paf::common::transition::DoReverse(
      0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack1,
      false, false);
}
*/

void paf_on_load(paf::Plugin *plugin) {

  g_samplePlugin = plugin;

  paf::Plugin::PageOpenParam pageOpenParam;
  paf::ui::Scene *pScene = plugin->PageOpen("page_main_bg", pageOpenParam);

  g_pReturnCornerButton = (paf::ui::CornerButton *)pScene->FindChild(
      "_sample_widget_corner_button_bottom_left");
  g_pReturnCornerButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                                          NULL);
  g_pReturnCornerButton->Hide(NULL, 2);
  g_pSettingsCornerButton = (paf::ui::CornerButton *)pScene->FindChild(
      "_sample_widget_corner_button_bottom_right");
  g_pSettingsCornerButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                                            NULL);
  g_pSettingsCornerButton->Hide(NULL, 2);

  g_pPlaneMain =
      (paf::ui::Plane *)pScene->FindChild("_sample_widget_plane_main");

  g_pPlaneRoot =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("_sample_widget_plane_root");

  paf::ui::Plane *pPlaneSub =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
  pPlaneSub->Hide(NULL, 2);

  paf::ui::ListView *list_view =
      (paf::ui::ListView *)g_pPlaneRoot->FindChild("list_view");

  auto itemFactory = new MenuItemFactory({{L"Unread articles", {}},
                                          {L"Favorited articles", {}},
                                          {L"Read articles", {}},
                                          {L"Settings", {}}});

  list_view->SetItemFactory(itemFactory);
  list_view->InsertSegment(0, 1);
  list_view->SetCellSizeDefault(0, {840.0f, 70.0f, 0.0f, 0.0f});

  // maybe add SetSegmentHeader here?
  list_view->SetSegmentLayoutType(0, paf::ui::ListView::LAYOUT_TYPE_LIST);
  list_view->InsertCell(0, 0, itemFactory->entries.size());
}

int paf_main(void) {

  paf::Framework::InitParam fwParam;
  fwParam.mode = paf::Framework::Mode_Normal;

  paf::Framework *paf_fw = new paf::Framework(fwParam);
  if (paf_fw != NULL) {
    g_fw = paf_fw;

    paf_fw->LoadCommonResourceSync();

    paf::Plugin::InitParam pluginParam;

    pluginParam.name = "main";
    pluginParam.caller_name = "__main__";
    pluginParam.resource_file = "app0:/main.rco";
    pluginParam.init_func = NULL;
    pluginParam.start_func = paf_on_load;
    pluginParam.stop_func = NULL;
    pluginParam.exit_func = NULL;

    paf::Plugin::LoadSync(pluginParam);
    paf_fw->Run();
  }

  exit(0);

  return 0;
}
char sceUserMainThreadName[] = "paf_sample";
int sceUserMainThreadPriority = 0x10000100;
int sceUserMainThreadCpuAffinityMask = 0x70000;
SceSize sceUserMainThreadStackSize = 0x4000;

void operator delete(void *ptr, unsigned int n) { return sce_paf_free(ptr); }

int paf_main(void);

extern "C" {

typedef struct _ScePafInit { // size is 0x18
  SceSize global_heap_size;
  int a2;
  int a3;
  int cdlg_mode;
  int heap_opt_param1;
  int heap_opt_param2;
} ScePafInit;

// void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {

  int load_res;
  ScePafInit init_param;
  SceSysmoduleOpt sysmodule_opt;

  init_param.global_heap_size = 0x1000000;
  init_param.a2 = 0xEA60;
  init_param.a3 = 0x40000;
  init_param.cdlg_mode = 0;
  init_param.heap_opt_param1 = 0;
  init_param.heap_opt_param2 = 0;

  load_res = 0xDEADBEEF;
  sysmodule_opt.flags = 0;
  sysmodule_opt.result = &load_res;

  int res = sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF,
                                                  sizeof(init_param),
                                                  &init_param, &sysmodule_opt);
  if ((res | load_res) != 0) {
    sceClibPrintf("[PAF PRX Loader] Failed to load the PAF prx. (return value "
                  "0x%x, result code 0x%x )\n",
                  res, load_res);
  }
  xml_document<wchar_t> document;

  paf_main();

  return SCE_KERNEL_START_SUCCESS;
}
}
