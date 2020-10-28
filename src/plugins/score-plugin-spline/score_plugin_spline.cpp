// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "score_plugin_spline.hpp"

#include <Process/GenericProcessFactory.hpp>
#include <Process/HeaderDelegate.hpp>
#include <Process/Inspector/ProcessInspectorWidgetDelegate.hpp>
#include <Process/ProcessFactory.hpp>
#include <Process/Style/ScenarioStyle.hpp>

#include <score/plugins/FactorySetup.hpp>
#include <score/plugins/StringFactoryKey.hpp>
#include <score/tools/std/HashMap.hpp>

#include <Spline/SplineExecution.hpp>
#include <Spline/SplineModel.hpp>
#include <Spline/SplinePresenter.hpp>
#include <Spline/SplineView.hpp>
#include <Process/Dataflow/Port.hpp>
#include <wobjectimpl.h>
namespace Spline
{
using SplineFactory = Process::ProcessFactory_T<Spline::ProcessModel>;
using SplineLayerFactory
    = Process::LayerFactory_T<Spline::ProcessModel, Spline::Presenter, Spline::View>;
}


score_plugin_spline::score_plugin_spline() = default;
score_plugin_spline::~score_plugin_spline() = default;

std::vector<std::unique_ptr<score::InterfaceBase>> score_plugin_spline::factories(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key) const
{
  return instantiate_factories<
      score::ApplicationContext,
      FW<Process::ProcessModelFactory,
         Spline::SplineFactory>,
      FW<Process::LayerFactory,
         Spline::SplineLayerFactory>,
      FW<Execution::ProcessComponentFactory,
         Spline::RecreateOnPlay::ComponentFactory>
      >(ctx, key);
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_plugin_spline)
