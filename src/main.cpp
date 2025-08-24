
#include <paf.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>

paf::Framework *g_fw;
paf::Plugin *g_samplePlugin;
paf::ui::Scene *g_rootPage;

int g_current = 0;
paf::ui::Plane *g_pPlane;
paf::ui::Plane *g_pPlaneMain;
paf::ui::Scene *g_pScene;
paf::ui::CornerButton *g_pReturnCornerButton;
paf::ui::Plane *g_pPlaneRoot;

static void button_handler(int32_t type, paf::ui::Handler *self,
                           paf::ui::Event *e, void *userdata) {
  // sceClibPrintf("Button pressed!\n");

  if ((g_current & 1) == 0) {
    paf::common::transition::Do(
        0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack1,
        false, false);

    paf::ui::Plane *pPlaneSub =
        (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
    pPlaneSub->Show(NULL, 2);
    paf::common::transition::Do(
        0.0f, pPlaneSub, paf::common::transition::Type::Type_3D_SlideFromFront,
        false, false);

    // paf::Plugin::TemplateOpenParam templateOpenParam;
    // g_samplePlugin->TemplateOpen(g_pScene,
    // paf::IDParam("_sample_template_main_page"), templateOpenParam);

    // paf::Plugin::PageOpenParam pageOpenParam;
    // g_pScene = g_samplePlugin->PageOpen(paf::IDParam("page_main_2"),
    // pageOpenParam);

    // paf::ui::Plane *pPlane = (paf::ui::Plane
    // *)g_pScene->FindChild("plane_sample_black");
    // pPlane->SetTransitionComplete(0);

    // paf::ui::Widget *pButton =
    // pPlane->FindChild("_sample_widget_corner_button_bottom_left");
    // pButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
    // button_handler);

    // pButton->Hide(NULL, 2);

    // paf::Timer *pTimer = new paf::Timer(200.0f,
    // paf::Timer::Func::FUNC_BOUNCE_IN); paf::Timer2D *pTimer2D = new
    // paf::Timer2D(200.0f, paf::Timer::Func::FUNC_QUAD_IN,
    // paf::Timer::Func::FUNC_QUAD_IN, 0, -1, NULL, NULL, 0);
    // pButton->Show(pTimer2D, 0, 500.0f);

    g_pReturnCornerButton->Show(NULL, 2);
    paf::common::transition::Do(0.0f, g_pReturnCornerButton,
                                paf::common::transition::Type::Type_Popup5,
                                false, false);
  } else {

    // paf::ui::Plane *pPlane = (paf::ui::Plane
    // *)g_pScene->FindChild("plane_sample_black");
    // pPlane->SetTransitionComplete(0);
    // paf::ui::Widget *pButton =
    // pPlane->FindChild("_sample_widget_corner_button_bottom_left");

    {
      paf::common::transition::DoReverse(
          0.0f, g_pReturnCornerButton,
          paf::common::transition::Type::Type_Popup5, false, false);
      paf::Timer *pTimer =
          new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN);
      g_pReturnCornerButton->Hide(pTimer, 2);
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
    // g_pScene->SetTransitionComplete(0);
    // paf::common::transition::DoReverse(400.0f, g_pScene,
    // paf::common::transition::Type::Type_3D_SlideFromFront, false, false);

    // paf::Plugin::PageCloseParam pageOpenParam;
    // g_samplePlugin->PageClose(paf::IDParam("page_main_2"), pageOpenParam);
  }

  g_current ^= 1;
}

paf::ui::Plane *pPlaneSystemUpdate;
paf::ui::ProgressBar *pProgressBarSystemUpdate;

int g_current_percentage = 0;

void percentage_return_HandlerCB(int32_t type, paf::ui::Handler *self,
                                 paf::ui::Event *e, void *userdata) {
  pPlaneSystemUpdate->Hide(
      new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN), 2);
  paf::common::transition::Do(
      0.0f, pPlaneSystemUpdate,
      paf::common::transition::Type::Type_3D_SlideToBack2, false, false);
  paf::common::transition::Do(
      0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideFromFront,
      false, false);
}

void percentage_HandlerCB(int32_t type, paf::ui::Handler *self,
                          paf::ui::Event *e, void *userdata) {

  if (g_current_percentage == 100) {
    pPlaneSystemUpdate->KillIntervalEvent(0);
    sceClibPrintf("%s\n", __FUNCTION__);
    pPlaneSystemUpdate->SetTimeoutEvent(1, 3000.0f,
                                        percentage_return_HandlerCB);
  }

  pProgressBarSystemUpdate->SetValue(g_current_percentage++, true);
}

void HandlerCB(int32_t type, paf::ui::Handler *self, paf::ui::Event *e,
               void *userdata) {
  sceClibPrintf("%s\n", __FUNCTION__);

  paf::ui::Plane *pPlaneSub2 =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub_2");
  paf::ui::Box *pBox = (paf::ui::Box *)pPlaneSub2->FindChild("box");
  paf::ui::BusyIndicator *pBusyIndicator =
      (paf::ui::BusyIndicator *)pBox->FindChild("busyindicator");
  pBusyIndicator->Stop();
  pPlaneSub2->Hide(new paf::Timer(500.0f, paf::Timer::Func::FUNC_BOUNCE_IN), 2);
  paf::common::transition::Do(
      0.0f, pPlaneSub2, paf::common::transition::Type::Type_3D_SlideToBack2,
      false, false);

  pPlaneSystemUpdate =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("_sample_widget_system_update");
  pPlaneSystemUpdate->Show(NULL, 2);
  paf::common::transition::Do(
      0.0f, pPlaneSystemUpdate,
      paf::common::transition::Type::Type_3D_SlideFromFront, false, false);

  pProgressBarSystemUpdate =
      (paf::ui::ProgressBar *)pPlaneSystemUpdate->FindChild("progressbar");
  pProgressBarSystemUpdate->SetMinValue(0);
  pProgressBarSystemUpdate->SetMaxValue(100);

  g_current_percentage = 0;
  pProgressBarSystemUpdate->SetValue(g_current_percentage);
  pPlaneSystemUpdate->SetIntervalEvent(0, 100.0f, percentage_HandlerCB);
}

static void button_handler_2(int32_t type, paf::ui::Handler *self,
                             paf::ui::Event *e, void *userdata) {
  paf::common::transition::Do(
      0.0f, g_pPlaneRoot, paf::common::transition::Type::Type_3D_SlideToBack2,
      false, false);

  paf::ui::Plane *pPlaneSub2 =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub_2");

  paf::ui::Box *pBox = (paf::ui::Box *)pPlaneSub2->FindChild("box");

  paf::ui::BusyIndicator *pBusyIndicator =
      (paf::ui::BusyIndicator *)pBox->FindChild("busyindicator");
  pBusyIndicator->Start();

  pPlaneSub2->Show(NULL, 2);
  paf::common::transition::Do(
      0.0f, pPlaneSub2, paf::common::transition::Type::Type_3D_SlideFromFront,
      false, false);

  pPlaneSub2->SetTimeoutEvent(0, 5000.0f, HandlerCB);
}

class my_ItemFactory : public paf::ui::listview::ItemFactory {
public:
  my_ItemFactory() {}

  ~my_ItemFactory() {}

  paf::ui::ListItem *Create(CreateParam &param);

  void Start(StartParam &param) {
    param.list_item->Show(paf::common::transition::Type_FadeinSlow);
  }

  void Stop(StopParam &param) {
    param.list_item->Hide(paf::common::transition::Type_FadeinSlow);
  }

  // void Dispose(DisposeParam& param){}
};

paf::ui::ListItem *my_ItemFactory::Create(CreateParam &param) {

  paf::Plugin::TemplateOpenParam openParam;
  int res = g_samplePlugin->TemplateOpen(
      param.parent, "_sample_template_list_item", openParam);
  if (res != 0) {
    sceClibPrintf("TemplateOpen 0x%X\n", res);
  }

  paf::ui::ListItem *list_item = (paf::ui::ListItem *)param.parent->GetChild(
      param.parent->GetChildrenNum() - 1);
  paf::ui::Widget *button = list_item->FindChild("button");

  switch (param.cell_index) {
  case 0:
    button->SetString(L"Like System Settings");
    button->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                             button_handler);
    break;
  case 1:
    button->SetString(L"Like System Update");
    button->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                             button_handler_2);
    break;
  default:
    button->SetString(L"Extra cell");
    break;
  }

  return list_item;
}

void loadPluginCB(paf::Plugin *plugin) {

  g_samplePlugin = plugin;

  paf::Plugin::PageOpenParam pageOpenParam;
  paf::ui::Scene *pScene = plugin->PageOpen("page_main_bg", pageOpenParam);

  g_pReturnCornerButton = (paf::ui::CornerButton *)pScene->FindChild(
      "_sample_widget_corner_button_bottom_left");
  g_pReturnCornerButton->SetEventCallback(paf::ui::ButtonBase::CB_BTN_DECIDE,
                                          button_handler);
  g_pReturnCornerButton->Hide(NULL, 2);

  g_pPlaneMain =
      (paf::ui::Plane *)pScene->FindChild("_sample_widget_plane_main");

  g_pPlaneRoot =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("_sample_widget_plane_root");

  paf::ui::Plane *pPlaneSub =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub");
  pPlaneSub->Hide(NULL, 2);

  paf::ui::Plane *pPlaneSub2 =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("plane_sample_black_sub_2");
  pPlaneSub2->Hide(NULL, 2);

  paf::ui::Plane *pPlaneSystemUpdate =
      (paf::ui::Plane *)g_pPlaneMain->FindChild("_sample_widget_system_update");
  pPlaneSystemUpdate->Hide(NULL, 2);

  paf::ui::ListView *list_view =
      (paf::ui::ListView *)g_pPlaneRoot->FindChild("list_view");
  list_view->SetItemFactory(new my_ItemFactory());
  list_view->InsertSegment(0, 1);
  list_view->SetCellSizeDefault(0, {840.0f, 70.0f, 0.0f, 0.0f});

  // maybe add SetSegmentHeader here?
  list_view->SetSegmentLayoutType(0, paf::ui::ListView::LAYOUT_TYPE_LIST);
  list_view->InsertCell(0, 0, 2);
}

int paf_sample_main(void) {

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
    pluginParam.start_func = loadPluginCB;
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

int paf_sample_main(void);

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

  paf_sample_main();

  // sceKernelExitProcess(0);

  return SCE_KERNEL_START_SUCCESS;
}
}
