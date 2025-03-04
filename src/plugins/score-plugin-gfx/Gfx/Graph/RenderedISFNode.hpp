#pragma once
#include <Gfx/Graph/ISFNode.hpp>
#include <Gfx/Graph/NodeRenderer.hpp>

#include <ossia/audio/fft.hpp>
#include <ossia/math/math_expression.hpp>
#include <ossia/detail/small_flat_map.hpp>

namespace score::gfx
{
struct Pass
{
  TextureRenderTarget renderTarget;
  Pipeline p;
  QRhiBuffer* processUBO{};
};

struct PersistSampler
{
  QRhiSampler* sampler{};
  QRhiTexture* textures[2]{nullptr, nullptr};
};
using PassOutput = std::variant<PersistSampler, TextureRenderTarget>;

struct AudioTextureUpload
{
  explicit AudioTextureUpload();

  void process(
      AudioTexture& audio,
      QRhiResourceUpdateBatch& res,
      QRhiTexture* rhiTexture);

  void processTemporal(
      AudioTexture& audio,
      QRhiResourceUpdateBatch& res,
      QRhiTexture* rhiTexture);

  void processSpectral(
      AudioTexture& audio,
      QRhiResourceUpdateBatch& res,
      QRhiTexture* rhiTexture);

  [[nodiscard]]
  std::optional<Sampler> updateAudioTexture(
      AudioTexture& audio,
      RenderList& renderer,
      QRhiResourceUpdateBatch& res);

private:
  std::vector<float> m_scratchpad;
  ossia::fft m_fft;
};

struct RenderedISFNode : score::gfx::NodeRenderer
{
  RenderedISFNode(const ISFNode& node) noexcept;

  virtual ~RenderedISFNode();

  TextureRenderTarget renderTargetForInput(const Port& p) override;

  void init(RenderList& renderer) override;
  void update(RenderList& renderer, QRhiResourceUpdateBatch& res) override;
  void release(RenderList& r) override;

  void runInitialPasses(
      RenderList&,
      QRhiCommandBuffer& commands,
      QRhiResourceUpdateBatch*& res,
      Edge& edge) override;

  void runRenderPass(
      RenderList&,
      QRhiCommandBuffer& commands,
      Edge& edge) override;

private:
  ossia::small_flat_map<const Port*, TextureRenderTarget, 2> m_rts;

  std::pair<Pass, Pass>
  createPass(RenderList& renderer,  ossia::small_vector<PassOutput, 1>& m_passSamplers, PassOutput target, bool previousPassIsPersistent);

  std::pair<Pass, Pass>
  createFinalPass(RenderList& renderer,  ossia::small_vector<PassOutput, 1>& m_passSamplers, const TextureRenderTarget& target);
  void initPasses(
      const TextureRenderTarget& rt,
      RenderList& renderer,
      Edge& edge,
      int& cur_pos,
      QSize mainTexSize
      );

  PassOutput initPassSampler(
      ISFNode& n,
      const isf::pass& pass,
      RenderList& renderer,
      int& cur_pos,
      QSize mainTexSize);

  struct Passes {
    ossia::small_vector<Pass, 1> passes;
    ossia::small_vector<Pass, 1> altPasses;
    ossia::small_vector<PassOutput, 1> samplers;
  };

  std::vector<Sampler> allSamplers(ossia::small_vector<PassOutput, 1>&, int mainOrAltPass) const noexcept;

  ossia::small_vector<std::pair<Edge*, Passes>, 2> m_passes;

  ISFNode& n;

  std::vector<Sampler> m_inputSamplers;
  std::vector<Sampler> m_audioSamplers;

  std::vector<TextureRenderTarget> m_innerPassTargets;

  QRhiBuffer* m_meshBuffer{};
  QRhiBuffer* m_idxBuffer{};

  QRhiBuffer* m_materialUBO{};
  int m_materialSize{};
  int64_t materialChangedIndex{-1};

  AudioTextureUpload m_audioTex;
};


// Used for the simple case of a single, non-persistent pass (the most common case)

struct SimpleRenderedISFNode : score::gfx::NodeRenderer
{
  SimpleRenderedISFNode(const ISFNode& node) noexcept;

  virtual ~SimpleRenderedISFNode();

  TextureRenderTarget renderTargetForInput(const Port& p) override;

  void init(RenderList& renderer) override;
  void update(RenderList& renderer, QRhiResourceUpdateBatch& res) override;
  void release(RenderList& r) override;

  void runInitialPasses(
      RenderList&,
      QRhiCommandBuffer& commands,
      QRhiResourceUpdateBatch*& res,
      Edge& edge) override;

  void runRenderPass(
      RenderList&,
      QRhiCommandBuffer& commands,
      Edge& edge) override;

private:
  ossia::small_flat_map<const Port*, TextureRenderTarget, 2> m_rts;

  void initPass(
      const TextureRenderTarget& rt,
      RenderList& renderer,
      Edge& edge
      );

  std::vector<Sampler> allSamplers() const noexcept;

  ossia::small_vector<std::pair<Edge*, Pass>, 2> m_passes;

  ISFNode& n;

  std::vector<Sampler> m_inputSamplers;
  std::vector<Sampler> m_audioSamplers;

  QRhiBuffer* m_meshBuffer{};
  QRhiBuffer* m_idxBuffer{};

  QRhiBuffer* m_materialUBO{};
  int m_materialSize{};
  int64_t materialChangedIndex{-1};

  AudioTextureUpload m_audioTex;
};
}
