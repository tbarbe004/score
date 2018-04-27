// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ObjectMenuActions.hpp"

#include "ScenarioCopy.hpp"
#include "TextDialog.hpp"

#include <Process/ProcessList.hpp>
#include <QAction>
#include <QByteArray>
#include <QClipboard>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenu>
#include <QObject>
#include <QRect>
#include <QString>
#include <QToolBar>
#include <Scenario/Application/ScenarioActions.hpp>
#include <Scenario/Application/ScenarioApplicationPlugin.hpp>
#include <Scenario/Application/ScenarioEditionSettings.hpp>
#include <Scenario/Commands/Cohesion/DoForSelectedIntervals.hpp>
#include <Scenario/Commands/Interval/InsertContentInInterval.hpp>
#include <Scenario/Commands/Scenario/Encapsulate.hpp>
#include <Scenario/Commands/Scenario/ScenarioPasteContent.hpp>
#include <Scenario/Commands/Scenario/ScenarioPasteElements.hpp>
#include <Scenario/Commands/State/InsertContentInState.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentModel.hpp>
#include <Scenario/Document/ScenarioDocument/ScenarioDocumentPresenter.hpp>
#include <Scenario/Palette/ScenarioPoint.hpp>
#include <Scenario/Process/Algorithms/ContainersAccessors.hpp>
#include <Scenario/Process/ScenarioGlobalCommandManager.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Process/Temporal/TemporalScenarioPresenter.hpp>
#include <Scenario/Process/Temporal/TemporalScenarioView.hpp>
#include <algorithm>
#include <core/application/ApplicationSettings.hpp>
#include <core/document/Document.hpp>
#include <qnamespace.h>
#include <score/actions/ActionManager.hpp>
#include <score/actions/Menu.hpp>
#include <score/actions/MenuManager.hpp>
#include <score/application/ApplicationContext.hpp>
#include <score/command/Dispatchers/MacroCommandDispatcher.hpp>
#include <score/document/DocumentInterface.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/path/Path.hpp>
#include <score/model/path/PathSerialization.hpp>
#include <score/selection/Selectable.hpp>
#include <score/serialization/DataStreamVisitor.hpp>
#include <score/tools/std/Optional.hpp>

namespace Scenario
{
ObjectMenuActions::ObjectMenuActions(ScenarioApplicationPlugin* parent)
    : m_parent{parent}
    , m_eventActions{parent}
    , m_cstrActions{parent}
    , m_stateActions{parent}
{
  if (!parent->context.applicationSettings.gui)
    return;
  using namespace score;

  // REMOVE
  m_removeElements = new QAction{tr("Remove selected elements"), this};
  m_removeElements->setShortcut(Qt::Key_Backspace); // NOTE : the effective
                                                    // shortcut is in
                                                    // ../ScenarioActions.hpp
  m_removeElements->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(m_removeElements, &QAction::triggered, [this]() {
    auto& ctx = m_parent->currentDocument()->context();

    auto sm = focusedScenarioModel(ctx);
    auto si = focusedScenarioInterface(ctx);

    if (sm)
    {
      Scenario::removeSelection(*sm, ctx.commandStack);
    }
    else if (si && !selectedElements(si->getStates()).empty())
    {
      Scenario::clearContentFromSelection(*si, ctx.commandStack);
    }
  });

  // COPY/CUT
  m_copyContent = new QAction{tr("Copy"), this};
  m_copyContent->setShortcut(QKeySequence::Copy);
  m_copyContent->setShortcutContext(Qt::ApplicationShortcut);
  connect(m_copyContent, &QAction::triggered, [this]() {
    auto obj = copySelectedElementsToJson();
    if (obj.empty())
      return;
    QJsonDocument doc{obj};
    auto clippy = QApplication::clipboard();
    clippy->setText(doc.toJson(QJsonDocument::Indented));
  });

  m_cutContent = new QAction{tr("Cut"), this};
  m_cutContent->setShortcut(QKeySequence::Cut);
  m_cutContent->setShortcutContext(Qt::ApplicationShortcut);
  connect(m_cutContent, &QAction::triggered, [this] {
    auto obj = cutSelectedElementsToJson();
    if (obj.empty())
      return;
    QJsonDocument doc{obj};
    auto clippy = QApplication::clipboard();
    clippy->setText(doc.toJson(QJsonDocument::Indented));
  });

  m_pasteContent = new QAction{tr("Paste content"), this};
  m_pasteContent->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(m_pasteContent, &QAction::triggered, [this] {
    writeJsonToSelectedElements(
        QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8())
            .object());
  });

  m_pasteElements = new QAction{tr("Paste elements"), this};
  m_pasteElements->setShortcut(QKeySequence::Paste);
  m_pasteElements->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(m_pasteElements, &QAction::triggered, [this] {
    auto pres = m_parent->focusedPresenter();
    if (!pres)
      return;
    auto views = pres->view().scene()->views();
    if (views.empty())
      return;

    auto view = views.front();

    QPoint pos = QCursor::pos();

    auto scene_pt = view->mapToScene(view->mapFromGlobal(pos));
    TemporalScenarioView& sv = pres->view();
    auto sv_pt = sv.mapFromScene(scene_pt);
    if (!sv.contains(sv_pt))
    {
      sv_pt = sv.mapToScene(sv.boundingRect().center());
    }
    auto pt = pres->toScenarioPoint(sv_pt);
    pasteElements(
        QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8())
            .object(),
        pt);
  });

  m_pasteElementsAfter = new QAction{tr("Paste (after)"), this};
  connect(m_pasteElementsAfter, &QAction::triggered, [this] {
    auto pres = m_parent->focusedPresenter();
    if (!pres)
      return;
    auto views = pres->view().scene()->views();
    if (views.empty())
      return;

    auto view = views.front();

    QPoint pos = QCursor::pos();

    auto scene_pt = view->mapToScene(view->mapFromGlobal(pos));
    TemporalScenarioView& sv = pres->view();
    auto sv_pt = sv.mapFromScene(scene_pt);
    if (!sv.contains(sv_pt))
    {
      sv_pt = sv.mapToScene(sv.boundingRect().center());
    }
    auto pt = pres->toScenarioPoint(sv_pt);
    pasteElementsAfter(
        QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8())
            .object(),
        pt, pres->context().context.selectionStack.currentSelection());
  });

  // DISPLAY JSON
  m_elementsToJson = new QAction{tr("Convert selection to JSON"), this};
  connect(m_elementsToJson, &QAction::triggered, [this]() {
    QJsonDocument doc{copySelectedElementsToJson()};
    TextDialog s{doc.toJson(QJsonDocument::Indented), qApp->activeWindow()};

    s.exec();
  });

  // MERGE TIMESYNC
  m_mergeTimeSyncs = new QAction{this};
  connect(m_mergeTimeSyncs, &QAction::triggered, [this]() {
    auto sm = focusedScenarioModel(m_parent->currentDocument()->context());
    SCORE_ASSERT(sm);

    Scenario::mergeTimeSyncs(
        *sm, m_parent->currentDocument()->context().commandStack);
  });

  // Merge events
  m_mergeEvents = new QAction{this};
  connect(m_mergeEvents, &QAction::triggered, [this]() {
    auto sm = focusedScenarioModel(m_parent->currentDocument()->context());
    if (!sm)
      return;

    Scenario::mergeEvents(
        *sm, m_parent->currentDocument()->context().commandStack);
  });

  // Encapsulate
  m_encapsulate = new QAction{this};
  connect(m_encapsulate, &QAction::triggered, [this]() {
    auto sm = focusedScenarioModel(m_parent->currentDocument()->context());
    SCORE_ASSERT(sm);

    Scenario::EncapsulateInScenario(
        *sm, m_parent->currentDocument()->context().commandStack);
  });

  // Duplicate
  m_duplicate = new QAction{this};
  connect(m_duplicate, &QAction::triggered, [this]() {
    auto sm = focusedScenarioModel(m_parent->currentDocument()->context());
    SCORE_ASSERT(sm);

    Scenario::Duplicate(
        *sm, m_parent->currentDocument()->context().commandStack);
  });

  // Selection actions
  m_selectAll = new QAction{tr("Select all"), this};
  m_selectAll->setToolTip("Ctrl+A");
  connect(m_selectAll, &QAction::triggered, [this]() {
    if (auto pres = getScenarioDocPresenter())
      pres->selectAll();
  });

  m_deselectAll = new QAction{tr("Deselect all"), this};
  m_deselectAll->setToolTip("Ctrl+Shift+A");
  connect(m_deselectAll, &QAction::triggered, [this]() {
    if (auto pres = getScenarioDocPresenter())
      pres->deselectAll();
  });

  m_selectTop = new QAction{this};
  connect(m_selectTop, &QAction::triggered, [this] {
    if (auto pres = getScenarioDocPresenter())
      pres->selectTop();
  });

  m_goToParent = new QAction{this};
  m_deselectAll->setToolTip("Ctrl+Shift+Up");
  connect(m_goToParent, &QAction::triggered, [this]() {
    if (auto pres = getScenarioDocPresenter())
    {
      auto* cur = (QObject*)&pres->displayedInterval();
      while((cur = cur->parent()))
      {
        if(auto parent = qobject_cast<Scenario::IntervalModel*>(cur))
        {
          pres->setDisplayedInterval(*parent);
          break;
        }
      }
    }
  });

  if (parent->context.mainWindow)
  {
    auto doc = parent->context.mainWindow->findChild<QWidget*>(
        "Documents", Qt::FindDirectChildrenOnly);
    SCORE_ASSERT(doc);
    doc->addAction(m_removeElements);
    doc->addAction(m_pasteElements);
    doc->addAction(m_goToParent);
  }
}

void ObjectMenuActions::makeGUIElements(score::GUIElements& e)
{
  using namespace score;
  auto& actions = e.actions;
  auto& base_menus = m_parent->context.menus.get();

  auto& scenariofocus_cond = m_parent->context.actions.condition<
      Process::EnableWhenFocusedProcessIs<Scenario::ProcessModel>>();
  auto& scenariomodel_cond
      = m_parent->context.actions.condition<EnableWhenScenarioModelObject>();
  auto& scenarioiface_cond
      = m_parent->context.actions
            .condition<EnableWhenScenarioInterfaceObject>();

  auto& scenariodocument_cond = m_parent->context.actions.condition<
      score::EnableWhenDocumentIs<Scenario::ScenarioDocumentModel>>();

  actions.add<Actions::RemoveElements>(m_removeElements);
  actions.add<Actions::CopyContent>(m_copyContent);
  actions.add<Actions::CutContent>(m_cutContent);
  actions.add<Actions::PasteContent>(m_pasteContent);
  actions.add<Actions::PasteElements>(m_pasteElements);
  actions.add<Actions::PasteElementsAfter>(m_pasteElementsAfter);
  actions.add<Actions::ElementsToJson>(m_elementsToJson);
  actions.add<Actions::MergeTimeSyncs>(m_mergeTimeSyncs);
  actions.add<Actions::MergeEvents>(m_mergeEvents);
  actions.add<Actions::Encapsulate>(m_encapsulate);
  actions.add<Actions::Duplicate>(m_duplicate);

  scenariomodel_cond.add<Actions::RemoveElements>();
  scenariomodel_cond.add<Actions::MergeTimeSyncs>();
  scenariomodel_cond.add<Actions::Encapsulate>();
  scenariofocus_cond.add<Actions::PasteElements>();

  scenarioiface_cond.add<Actions::CopyContent>();
  scenarioiface_cond.add<Actions::CutContent>();
  scenarioiface_cond.add<Actions::PasteContent>();
  scenarioiface_cond.add<Actions::ElementsToJson>();

  Menu& object = base_menus.at(Menus::Object());
  object.menu()->addAction(m_elementsToJson);
  object.menu()->addAction(m_removeElements);
  object.menu()->addSeparator();
  object.menu()->addAction(m_copyContent);
  object.menu()->addAction(m_cutContent);
  object.menu()->addAction(m_pasteContent);
  object.menu()->addAction(m_pasteElements);
  object.menu()->addAction(m_pasteElementsAfter);
  object.menu()->addSeparator();
  object.menu()->addAction(m_mergeTimeSyncs);
  object.menu()->addAction(m_mergeEvents);
  object.menu()->addAction(m_encapsulate);
  object.menu()->addAction(m_duplicate);
  m_eventActions.makeGUIElements(e);
  m_cstrActions.makeGUIElements(e);
  m_stateActions.makeGUIElements(e);

  Menu& view = base_menus.at(Menus::View());
  view.menu()->addAction(m_selectAll);
  view.menu()->addAction(m_deselectAll);
  view.menu()->addAction(m_selectTop);
  view.menu()->addAction(m_goToParent);

  e.actions.add<Actions::SelectAll>(m_selectAll);
  e.actions.add<Actions::DeselectAll>(m_deselectAll);
  e.actions.add<Actions::SelectTop>(m_selectTop);
  e.actions.add<Actions::GoToParent>(m_goToParent);

  scenariodocument_cond.add<Actions::SelectAll>();
  scenariodocument_cond.add<Actions::DeselectAll>();
  scenariodocument_cond.add<Actions::SelectTop>();
  scenariodocument_cond.add<Actions::GoToParent>();
}

void ObjectMenuActions::setupContextMenu(
    Process::LayerContextMenuManager& ctxm)
{
  using namespace Process;
  LayerContextMenu scenario_model
      = MetaContextMenu<ContextMenus::ScenarioModelContextMenu>::make();
  LayerContextMenu scenario_object
      = MetaContextMenu<ContextMenus::ScenarioObjectContextMenu>::make();

  // Used for scenario model
  scenario_model.functions.push_back([this](
                                         QMenu& menu, QPoint,
                                         QPointF scenePoint,
                                         const LayerContext& ctx) {
    auto& scenario
        = *safe_cast<const TemporalScenarioPresenter*>(&ctx.presenter);
    auto sel = ctx.context.selectionStack.currentSelection();
    if (!sel.empty())
    {
      auto objectMenu = menu.addMenu(tr("Object"));

      objectMenu->addAction(m_elementsToJson);
      objectMenu->addAction(m_removeElements);
      objectMenu->addSeparator();

      objectMenu->addAction(m_copyContent);
      objectMenu->addAction(m_cutContent);
      objectMenu->addAction(m_pasteContent);
      objectMenu->addAction(m_encapsulate);
      objectMenu->addAction(m_duplicate);
    }

    auto pasteElements = new QAction{tr("Paste element(s)"), this};
    pasteElements->setShortcut(QKeySequence::Paste);
    pasteElements->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteElements, &QAction::triggered, [&, scenePoint]() {
      this->pasteElements(
          QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8())
              .object(),
          scenario.toScenarioPoint(scenario.view().mapFromScene(scenePoint)));
    });
    menu.addAction(pasteElements);

    menu.addAction(m_pasteElementsAfter);
  });

  // Used for base scenario, loops, etc.
  scenario_object.functions.push_back(
      [this](QMenu& menu, QPoint, QPointF, const LayerContext& ctx) {
        auto sel = ctx.context.selectionStack.currentSelection();
        if (!sel.empty())
        {
          auto objectMenu = menu.addMenu(tr("Object"));

          objectMenu->addAction(m_elementsToJson);
          objectMenu->addSeparator();

          objectMenu->addAction(m_copyContent);
          objectMenu->addAction(m_pasteContent);
        }
      });
  m_eventActions.setupContextMenu(ctxm);
  m_cstrActions.setupContextMenu(ctxm);
  m_stateActions.setupContextMenu(ctxm);

  ctxm.insert(std::move(scenario_model));
  ctxm.insert(std::move(scenario_object));
}

QJsonObject ObjectMenuActions::copySelectedElementsToJson()
{
  auto si = focusedScenarioInterface(m_parent->currentDocument()->context());
  auto si_obj = dynamic_cast<QObject*>(const_cast<ScenarioInterface*>(si));
  if (auto sm = dynamic_cast<const Scenario::ProcessModel*>(si))
  {
    return copySelectedScenarioElements(*sm);
  }
  else if (auto bsm = dynamic_cast<const Scenario::BaseScenarioContainer*>(si))
  {
    return copySelectedScenarioElements(*bsm, si_obj);
  }
  else
  {
    // Full-view copy
    auto& bem
        = score::IDocument::modelDelegate<Scenario::ScenarioDocumentModel>(
            *m_parent->currentDocument());
    if (!bem.baseScenario().selectedChildren().empty())
    {
      return copySelectedScenarioElements(
          bem.baseScenario(), &bem.baseScenario());
    }
  }

  return {};
}

QJsonObject ObjectMenuActions::cutSelectedElementsToJson()
{
  auto obj = copySelectedElementsToJson();
  if (obj.empty())
    return {};

  auto& ctx = m_parent->currentDocument()->context();

  if (auto sm = focusedScenarioModel(ctx))
  {
    Scenario::removeSelection(*sm, ctx.commandStack);
  }
  else if (auto si = focusedScenarioInterface(ctx))
  {
    Scenario::clearContentFromSelection(*si, ctx.commandStack);
  }

  return obj;
}

void ObjectMenuActions::pasteElements(
    const QJsonObject& obj, const Scenario::Point& origin)
{
  // TODO check for unnecessary uses of focusedProcessModel after
  // focusedPresenter.
  auto pres = m_parent->focusedPresenter();
  if (!pres)
    return;

  auto& sm = static_cast<const Scenario::ProcessModel&>(pres->model());
  // TODO check json validity
  auto cmd = new Command::ScenarioPasteElements(sm, obj, origin);

  dispatcher().submitCommand(cmd);
}

void ObjectMenuActions::pasteElementsAfter(
    const QJsonObject& obj,
    const Scenario::Point& origin,
    const Selection& sel)
{
  // TODO check for unnecessary uses of focusedProcessModel after
  // focusedPresenter.
  auto pres = m_parent->focusedPresenter();
  if (!pres)
    return;

  auto& sm = static_cast<const Scenario::ProcessModel&>(pres->model());
  // TODO check json validity
  auto cmd = new Command::ScenarioPasteElementsAfter{sm, obj};

  dispatcher().submitCommand(cmd);
}

template <typename Scenario_T>
static void writeJsonToScenario(
    const Scenario_T& scen,
    const ObjectMenuActions& self,
    const QJsonObject& obj)
{
  MacroCommandDispatcher<Command::ScenarioPasteContent> dispatcher{
      self.dispatcher().stack()};
  auto selectedIntervals = selectedElements(getIntervals(scen));
  auto expandMode = self.appPlugin()->editionSettings().expandMode();
  for (const auto& json_vref : obj["Intervals"].toArray())
  {
    for (const auto& interval : selectedIntervals)
    {
      auto cmd = new Scenario::Command::InsertContentInInterval{
          json_vref.toObject(), *interval, expandMode};

      dispatcher.submitCommand(cmd);
    }
  }

  auto selectedStates = selectedElements(getStates(scen));
  for (const auto& json_vref : obj["States"].toArray())
  {
    for (const auto& state : selectedStates)
    {
      auto cmd
          = new Command::InsertContentInState{json_vref.toObject(), *state};

      dispatcher.submitCommand(cmd);
    }
  }

  dispatcher.commit();
}

void ObjectMenuActions::writeJsonToSelectedElements(const QJsonObject& obj)
{
  auto si = focusedScenarioInterface(m_parent->currentDocument()->context());
  if (auto sm = dynamic_cast<const Scenario::ProcessModel*>(si))
  {
    writeJsonToScenario(*sm, *this, obj);
  }
  else if (auto bsm = dynamic_cast<const Scenario::BaseScenarioContainer*>(si))
  {
    writeJsonToScenario(*bsm, *this, obj);
  }
  else
  {
    SCORE_TODO;
    /*
    // Full-view paste
    auto& bem =
    score::IDocument::modelDelegate<ScenarioDocumentModel>(*m_parent->currentDocument());
    if(bem.baseInterval().selection.get())
    {
        return copySelectedScenarioElements(bem.baseScenario());
    }*/
  }
}

ScenarioDocumentModel* ObjectMenuActions::getScenarioDocModel() const
{
  if (auto doc = m_parent->currentDocument())
    return score::IDocument::try_get<ScenarioDocumentModel>(*doc);
  return nullptr;
}

ScenarioDocumentPresenter* ObjectMenuActions::getScenarioDocPresenter() const
{
  if (auto doc = m_parent->currentDocument())
    return score::IDocument::try_get<ScenarioDocumentPresenter>(*doc);
  return nullptr;
}

CommandDispatcher<> ObjectMenuActions::dispatcher() const
{
  CommandDispatcher<> disp{
      m_parent->currentDocument()->context().commandStack};
  return disp;
}
}
