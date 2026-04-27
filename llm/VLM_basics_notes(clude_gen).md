# VLM / LLM 完整知识笔记
> 面向大模型 / AI-Infra 岗位 · 涵盖模型结构、训练推理框架、后训练与部署应用
> 持续更新 · 最后同步：2025-04

---

## 目录

1. [模型结构总体框架](#1-模型结构总体框架)
2. [归一化 · 残差 · 激活函数 · 损失函数](#2-归一化--残差--激活函数--损失函数)
3. [优化器](#3-优化器)
4. [位置编码策略](#4-位置编码策略)
5. [Attention 改进：MHA → GQA → MLA](#5-attention-改进mha--gqa--mla)
6. [KV Cache 与显存计算](#6-kv-cache-与显存计算)
7. [FFN → MoE 及路由改进](#7-ffn--moe-及路由改进)
8. [多模态视觉融合：CLIP 及 VLM 架构](#8-多模态视觉融合clip-及-vlm-架构)
9. [大模型训练与推理框架](#9-大模型训练与推理框架)
10. [模型后训练：SFT · LoRA · RLHF · GRPO](#10-模型后训练sft--lora--rlhf--grpo)
11. [模型蒸馏与量化](#11-模型蒸馏与量化)
12. [Agentic AI：训练框架与推理框架](#12-agentic-ai训练框架与推理框架)
13. [重点论文与项目索引](#13-重点论文与项目索引)

---

## 1. 模型结构总体框架

### 1.1 Transformer 整体结构

原始 Transformer（Vaswani et al., 2017，[arXiv 1706.03762](https://arxiv.org/pdf/1706.03762)）采用 Encoder-Decoder 架构。现代 LLM 通常仅使用 **Decoder-only** 结构，并在此基础上做以下改动：

| 组件 | 原始 Transformer | 现代 LLM（如 LLaMA/Qwen/DeepSeek） |
|---|---|---|
| Normalization | Post-LN（残差后归一化） | Pre-RMSNorm（残差前归一化） |
| Activation | ReLU | SiLU / SwiGLU |
| Position Encoding | 绝对正弦位置编码 | RoPE |
| Attention | MHA | GQA / MLA |
| FFN | Dense FFN | Dense FFN 或 MoE |

**Decoder-only 单层结构**（以 Pre-Norm 为例）：

```
x → RMSNorm → Multi-Head Attention → (+residual) → RMSNorm → FFN → (+residual) → x_out
```

### 1.2 Transformer Multi-Head Attention 时间复杂度推导

设序列长度为 $n$，隐层维度为 $d$，头数为 $h$，每头维度 $d_h = d/h$。

**① Q, K, V 投影：** $n \times d$ 乘以 $d \times d_h$，共 $h$ 头：
$$\text{复杂度} = O(h \cdot n d \cdot d_h) = O(nd^2)$$

**② Attention 得分 $QK^\top$：** 将张量视为 $h \times n \times d_h$ 与 $h \times d_h \times n$：
$$\text{复杂度} = O(h \cdot n \cdot d_h \cdot n) = O(n^2 d)$$

**③ Softmax：** 对 $h$ 个 $n \times n$ 矩阵的每行做归一化：
$$\text{复杂度} = O(hn^2)$$

**④ 加权求和 $AV$：** $h \times n \times n$ 乘以 $h \times n \times d_h$：
$$\text{复杂度} = O(n^2 d)$$

**⑤ 输出投影 $W_O$：** $n \times d$ 乘以 $d \times d$：
$$\text{复杂度} = O(nd^2)$$

**汇总：**
$$\boxed{T_{\text{MHA}} = O(n^2 d + nd^2)}$$

当 $n \ll d$ 时，参数计算主导（$nd^2$）；当 $n \gg d$ 时，注意力计算主导（$n^2 d$）。

### 1.3 Vision Transformer（ViT）

ViT（Dosovitskiy et al., 2020，[arXiv 2010.11929](https://arxiv.org/pdf/2010.11929)）将图像切分为 $P \times P$ 的 patch，展平后线性映射为 token embedding，随后送入标准 Transformer Encoder。

- 图像尺寸 $H \times W$，patch 大小 $P \times P$，序列长度 $N = HW/P^2$
- 额外增加 `[CLS]` token 用于分类
- 使用可学习的绝对位置编码（1D）

$$z_0 = [x_{\text{cls}}; x_1^p E; x_2^p E; \ldots; x_N^p E] + E_{\text{pos}}$$

**ViT 局限性：** 在中小数据集上需要大量正则化；对细粒度局部特征感知弱（没有卷积归纳偏置）。

---

## 2. 归一化 · 残差 · 激活函数 · 损失函数

### 2.1 归一化方法对比

#### Batch Normalization（BN）

BN 沿 batch 维度归一化，解决内部协变量偏移（Internal Covariate Shift）：

$$\mu_B = \frac{1}{m}\sum_{i=1}^m x_i, \quad \sigma_B^2 = \frac{1}{m}\sum_{i=1}^m (x_i-\mu_B)^2$$
$$\hat{x}_i = \frac{x_i - \mu_B}{\sqrt{\sigma_B^2 + \epsilon}}, \quad y_i = \gamma \hat{x}_i + \beta$$

**优点：** 加速收敛、缓解梯度消失/爆炸，有一定正则化效果。  
**缺点：** 依赖 batch size；NLP 场景中序列变长，batch 统计不稳定；推理时需维护运行均值/方差。

#### Layer Normalization（LN）

沿特征维度（同一样本的所有特征）归一化，适用于 NLP/Transformer：

$$\mu = \frac{1}{H}\sum_{i=1}^H x_i, \quad \sigma = \sqrt{\frac{1}{H}\sum_{i=1}^H(x_i-\mu)^2 + \epsilon}$$
$$y = \frac{x-\mu}{\sigma} \cdot \gamma + \beta$$

独立于 batch size，适合变长序列。

#### RMS Norm

去除 LN 的均值中心化步骤，只保留尺度归一化，计算速度更快（LLaMA, Qwen, DeepSeek 均采用）：

$$\text{RMS}(x) = \sqrt{\frac{1}{H}\sum_{i=1}^H x_i^2 + \epsilon}$$
$$\bar{x}_i = \frac{x_i}{\text{RMS}(x)} \cdot \gamma_i$$

**理论依据：** 均值中心化对梯度流的贡献有限，去除后精度基本不变，计算量减少约 15%。

```python
class RMSNorm(nn.Module):
    def __init__(self, dim: int, eps: float = 1e-6):
        super().__init__()
        self.eps = eps
        self.weight = nn.Parameter(torch.ones(dim))   # γ，初始化为1

    def _norm(self, x):
        return x * torch.rsqrt(x.pow(2).mean(-1, keepdim=True) + self.eps)

    def forward(self, x):
        return self._norm(x.float()).type_as(x) * self.weight
```

#### Deep Norm（DeepNet）

DeepNet（Wang et al., 2022）提出在残差连接前对子层输出做 $\alpha$ 缩放，搭配初始化缩放 $\beta$，使训练深度网络（1000层+）更稳定：

$$x_{l+1} = \text{LN}(\alpha \cdot x_l + G(x_l, \theta_l))$$

其中 $\alpha$ 和 $\beta$ 根据网络深度 $N$ 和宽度 $M$ 推导自 update ratio 上界，保证梯度范数有界。

#### Pre-Norm vs. Post-Norm

| 位置 | 公式 | 特点 |
|---|---|---|
| Post-LN（原始 Transformer） | $x_{l+1} = \text{LN}(x_l + F(x_l))$ | 训练更稳定但梯度消失风险大 |
| Pre-LN（现代 LLM） | $x_{l+1} = x_l + F(\text{LN}(x_l))$ | 梯度流更顺畅，易于深层训练 |
| Sandwich-LN | Pre-LN + Post-LN 双重归一化 | 部分工作发现能进一步稳定训练 |

### 2.2 残差连接

残差连接（He et al., ResNet）解决深层网络梯度消失：

$$y = F(x, \{W_i\}) + x$$

在 Transformer 中，每个子层（Attention、FFN）后均有残差连接。其核心作用：
- 提供梯度直通路径（identity shortcut），避免梯度消失
- 使网络可选择性地学习增量（残差），而非重新学习恒等映射
- 从信息论角度看，允许浅层信息直接传递到深层

### 2.3 激活函数

#### ReLU 及其问题

$$\text{ReLU}(x) = \max(0, x)$$

优点：计算简单，缓解梯度消失。  
缺点：**Dead Neuron** 问题（负输入区梯度恒为 0）；不以 0 为中心。

#### SiLU（Swish）

$$\text{SiLU}(x) = x \cdot \sigma(x) = \frac{x}{1+e^{-x}}$$

由 Google Brain 提出，平滑、无饱和区域，在 LLM 中普遍优于 ReLU/GeLU。

#### GeLU

$$\text{GeLU}(x) = x \cdot \Phi(x) \approx 0.5x\left(1 + \tanh\left[\sqrt{\frac{2}{\pi}}\left(x + 0.044715x^3\right)\right]\right)$$

BERT、GPT 系列广泛使用。

#### SwiGLU（LLaMA, Qwen, Gemma 等主流 LLM）

SwiGLU 是 Gated Linear Unit（GLU）的 SiLU 变体，由 Noam Shazeer 提出，被证明在 LLM 中效果更好：

$$\text{SwiGLU}(x, W, V, b, c) = \text{SiLU}(xW + b) \odot (xV + c)$$

在 FFN 中的具体形式（不含偏置）：

$$\text{FFN}_{\text{SwiGLU}}(x) = \text{SiLU}(xW_1) \odot (xW_3) \cdot W_2$$

其中 $W_1, W_3 \in \mathbb{R}^{d \times d_{\text{ff}}}$，$W_2 \in \mathbb{R}^{d_{\text{ff}} \times d}$。  
注意：为保持参数量不变，$d_{\text{ff}}$ 通常取为 $\frac{2}{3} \cdot 4d$（约 $2.67d$）。

#### 激活函数对比

| 激活函数 | 公式 | 主要使用模型 |
|---|---|---|
| ReLU | $\max(0,x)$ | ResNet, 早期 Transformer |
| GeLU | $x\Phi(x)$ | BERT, GPT-2/3 |
| SiLU / Swish | $x\sigma(x)$ | EfficientNet |
| SwiGLU | $\text{SiLU}(xW_1)\odot(xV)$ | LLaMA, Qwen, DeepSeek |

### 2.4 损失函数

#### 语言模型预训练：交叉熵损失

$$\mathcal{L}_{\text{CLM}} = -\frac{1}{T}\sum_{t=1}^T \log P_\theta(x_t | x_{<t})$$

即在序列中每个位置预测下一个 token 的负对数似然之和（next-token prediction）。

#### Multi-Token Prediction（MTP，DeepSeek-V3）

延伸预测目标到未来 $k$ 个 token，提升数据利用效率：

$$\mathcal{L}_{\text{MTP}} = -\frac{1}{T}\sum_{t=1}^T \sum_{i=1}^k \lambda_i \log P_\theta(x_{t+i} | x_{\leq t})$$

通过串联多个 MTP 头（共享主干，各有独立输出投影）实现，推理时仅使用主头。

#### 对比学习损失（CLIP）

InfoNCE / NT-Xent：

$$\mathcal{L}_{\text{CLIP}} = -\frac{1}{N}\sum_{i=1}^N \log \frac{\exp(\text{sim}(I_i, T_i)/\tau)}{\sum_{j=1}^N \exp(\text{sim}(I_i, T_j)/\tau)}$$

其中 $\text{sim}$ 为余弦相似度，$\tau$ 为可学习温度参数。

---

## 3. 优化器

### 3.1 发展脉络

$$\text{SGD} \xrightarrow{+\text{动量}} \text{SGD-M} \xrightarrow{+\text{自适应学习率}} \text{AdaGrad/RMSProp} \xrightarrow{+\text{两者结合}} \text{Adam} \xrightarrow{+\text{解耦权重衰减}} \text{AdamW}$$

### 3.2 SGD with Momentum

$$v_t = \beta_1 v_{t-1} + (1-\beta_1)g_t$$
$$\theta_t = \theta_{t-1} - \eta v_t$$

动量项 $v_t$ 积累了历史梯度方向，能有效抑制震荡、加速鞍点逃离。

### 3.3 RMSProp

$$s_t = \gamma s_{t-1} + (1-\gamma)g_t^2$$
$$\theta_t = \theta_{t-1} - \frac{\eta}{\sqrt{s_t + \epsilon}} g_t$$

自适应缩放学习率：历史梯度较大的参数使用更小的有效学习率，解决 AdaGrad 学习率单调递减问题。

### 3.4 Adam

结合 Momentum（一阶矩）和 RMSProp（二阶矩），并引入偏差修正：

$$m_t = \beta_1 m_{t-1} + (1-\beta_1)g_t \quad \text{（一阶矩，动量）}$$
$$v_t = \beta_2 v_{t-1} + (1-\beta_2)g_t^2 \quad \text{（二阶矩，未中心化方差）}$$
$$\hat{m}_t = \frac{m_t}{1-\beta_1^t}, \quad \hat{v}_t = \frac{v_t}{1-\beta_2^t} \quad \text{（偏差修正）}$$
$$\theta_t = \theta_{t-1} - \eta \frac{\hat{m}_t}{\sqrt{\hat{v}_t}+\epsilon}$$

典型超参：$\beta_1=0.9$，$\beta_2=0.999$，$\epsilon=10^{-8}$。

**Adam 的问题：** L2 正则化与 weight decay 不等价，导致正则化效果偏弱。

### 3.5 AdamW（现代 LLM 标准优化器）

将权重衰减从梯度更新中解耦，直接作用于参数：

$$\theta_t = \theta_{t-1} - \eta \left(\frac{\hat{m}_t}{\sqrt{\hat{v}_t}+\epsilon} + \lambda \theta_{t-1}\right)$$

**优点：** 正则化效果更稳定，训练大模型时表现更好（Loshchilov & Hutter, 2017）。

### 3.6 Lion（EvoLved Sign Momentum）

Google Brain 提出，通过符号更新代替自适应学习率，内存占用低于 Adam（只需存一阶矩）：

$$c_t = \beta_1 m_{t-1} + (1-\beta_1)g_t$$
$$\theta_t = \theta_{t-1} - \eta \cdot \text{sign}(c_t) - \eta\lambda\theta_{t-1}$$
$$m_t = \beta_2 m_{t-1} + (1-\beta_2)g_t$$

在视觉模型上有时优于 AdamW，但在 LLM 上效果尚有争议。

### 3.7 Muon（2025 前沿）

Kosson et al. 提出，通过 Nesterov 动量 + 正交化（Newton-Schulz 迭代）使更新矩阵保持正交，在 MLP/Attention 权重矩阵上效果出色，被 Modular AI 等团队用于小规模 LLM 训练。

---

## 4. 位置编码策略

### 4.1 绝对正弦位置编码

$$PE_{(\text{pos}, 2i)} = \sin\!\left(\frac{\text{pos}}{10000^{2i/d}}\right), \quad PE_{(\text{pos}, 2i+1)} = \cos\!\left(\frac{\text{pos}}{10000^{2i/d}}\right)$$

优点：无可学习参数，可外推到训练时未见长度。  
缺点：无法有效捕获相对位置关系；现代 LLM 中已基本被 RoPE 取代。

### 4.2 相对位置编码

以 T5 Relative Bias 为代表，在 Attention 得分中加入可学习的相对位置偏置 $b_{m-n}$：

$$a_{mn} = \frac{(q_m + \Delta_m)(k_n + \Delta_n)^\top}{\sqrt{d}} + b_{m-n}$$

### 4.3 旋转位置编码（RoPE）

RoPE（Su et al., 2021）核心思想：**不将位置信息加入 embedding，而是在计算 Attention 时通过旋转矩阵将位置信息注入 Q/K**，使内积自然地只包含相对位置 $m-n$。

#### 推导

设 $q_m, k_n \in \mathbb{R}^d$，希望内积满足：

$$\langle f(q, m), f(k, n) \rangle = g(q, k, m-n)$$

对 2D 向量对 $(q_{2i}, q_{2i+1})$，旋转角度为 $m\theta_i$，$\theta_i = 10000^{-2i/d}$：

$$f(q, m) = \begin{pmatrix} \cos m\theta_i & -\sin m\theta_i \\ \sin m\theta_i & \cos m\theta_i \end{pmatrix} \begin{pmatrix} q_{2i} \\ q_{2i+1} \end{pmatrix}$$

合并成完整 $d$ 维向量的旋转矩阵形式：

$$R_m = \text{diag}\left(R_{m,0}, R_{m,1}, \ldots, R_{m,d/2-1}\right), \quad R_{m,i} = \begin{pmatrix} \cos m\theta_i & -\sin m\theta_i \\ \sin m\theta_i & \cos m\theta_i \end{pmatrix}$$

因此：

$$\langle R_m q, R_n k\rangle = q^\top R_m^\top R_n k = q^\top R_{m-n} k$$

只依赖相对位置 $m-n$，满足我们的要求。

#### 快速计算（无需显式构造旋转矩阵）

利用复数表示：$q_{2i}+jq_{2i+1}$ 乘以 $e^{jm\theta_i}$，等价于向量旋转。实现时：

$$\text{RoPE}(q, m)_{2i} = q_{2i}\cos(m\theta_i) - q_{2i+1}\sin(m\theta_i)$$
$$\text{RoPE}(q, m)_{2i+1} = q_{2i}\sin(m\theta_i) + q_{2i+1}\cos(m\theta_i)$$

```python
def apply_rotary_emb(x, cos, sin):
    # x: (B, H, T, D)
    x1 = x[..., ::2]
    x2 = x[..., 1::2]
    # 将配对后的维度旋转
    rotated = torch.stack([-x2, x1], dim=-1).flatten(-2)
    return x * cos + rotated * sin
```

#### RoPE 的外推与扩展

- **YaRN（Yet another RoPE extensioN）：** 对不同频率分量分别处理，高频维度直接外推，低频维度做线性缩放，在长文本任务上外推效果优于原始 RoPE
- **LongRoPE：** 对 $\theta$ 进行非均匀缩放，通过搜索最优缩放因子扩展上下文窗口
- **Dynamic NTK Scaling（Llama 2/3）：** 在推理时根据实际序列长度动态调整 base 频率

### 4.4 M-RoPE（Qwen2.5-VL 多模态位置编码）

针对多模态输入（图像+视频+文本），每个 token 携带三维位置信息（时间 $t$、高度 $h$、宽度 $w$）：

$$\text{MRoPE}(x, t, h, w) = \text{RoPE}_t(x) \oplus \text{RoPE}_h(x) \oplus \text{RoPE}_w(x)$$

将 $d_{\text{head}}$ 维度三等分，分别编码时间、高度、宽度的旋转：

$$q' = \text{RoPE}(q_{:d/3}, t) \| \text{RoPE}(q_{d/3:2d/3}, h) \| \text{RoPE}(q_{2d/3:}, w)$$

对纯文本 token，令 $t=h=w=\text{pos}$，退化为标准 1D RoPE，与文本-图像 token 的位置编码在同一空间内一致对齐。

**优势：**
- 统一图像（$t$ 固定）、视频（$t$ 变化）和文本的位置编码
- 明确建模空间关系（高度 $h$、宽度 $w$），增强对图像细节的理解
- 视频帧间时序关系通过 $t$ 维度自然表达

---

## 5. Attention 改进：MHA → GQA → MLA

### 5.1 MHA（Multi-Head Attention）

标准多头注意力，$n_h$ 个独立的 Q/K/V 头：

$$\text{head}_i = \text{softmax}\!\left(\frac{Q_i K_i^\top}{\sqrt{d_h}}\right) V_i$$
$$\text{MHA}(Q,K,V) = \text{Concat}(\text{head}_1,\ldots,\text{head}_{n_h}) W^O$$

**KV Cache 大小（每 token）：** $2 \cdot l \cdot n_h \cdot d_h$ 个元素。

### 5.2 MQA（Multi-Query Attention，Shazeer 2019）

所有 Q 头共享同一组 K/V 头（1 个 K/V）：

**KV Cache 大小（每 token）：** $2 \cdot l \cdot d_h$（减少 $n_h$ 倍）。  
**问题：** 模型容量下降，性能明显退化。

### 5.3 GQA（Grouped-Query Attention，Ainslie et al. 2023）

将 $n_h$ 个 Q 头分为 $g$ 组，每组共享 1 个 K/V 头：

$$g = n_h / n_{\text{kv\_heads}}, \quad 1 \leq g \leq n_h$$

**KV Cache 大小（每 token）：** $2 \cdot l \cdot n_{\text{kv\_heads}} \cdot d_h$。  
LLaMA-2/3、Qwen2、Mistral 均采用 GQA（典型 $n_{\text{kv\_heads}} = n_h/8$ 或 $n_h/4$）。

**KV 头数对比：**

| 方法 | KV 头数 | KV Cache 大小（每 token） |
|---|---|---|
| MHA | $n_h$ | $2 l n_h d_h$ |
| GQA | $g \ll n_h$ | $2 l g d_h$ |
| MQA | 1 | $2 l d_h$ |
| MLA | 低秩压缩 | $2 l (d_c + d_r^h)$（见下） |

### 5.4 MLA（Multi-Head Latent Attention，DeepSeek-V2/V3）

MLA 的核心思想是**低秩联合压缩 KV**，在减少 KV Cache 的同时不损失模型建模能力（与 GQA/MQA 不同，不减少参数）。

#### KV 低秩压缩

对输入 $h_t$ 做降维投影，得到潜在向量 $c_t^{KV}$（维度 $d_c \ll d$）：

$$c_t^{KV} = h_t W^{DKV} \quad (W^{DKV} \in \mathbb{R}^{d \times d_c}, \text{Down-proj})$$

再从潜在向量展开出 K/V：

$$k_t^C = c_t^{KV} W^{UK} \quad (W^{UK} \in \mathbb{R}^{d_c \times n_h d_h})$$
$$v_t^C = c_t^{KV} W^{UV} \quad (W^{UV} \in \mathbb{R}^{d_c \times n_h d_h})$$

缓存 $c_t^{KV}$ 而非全量 K/V，显存占用从 $2n_h d_h$ 降至 $d_c$。

#### Q 低秩压缩（减少激活显存）

$$c_t^Q = h_t W^{DQ} \in \mathbb{R}^{d_c'}, \quad q_t^C = c_t^Q W^{UQ} \in \mathbb{R}^{n_h d_h}$$

#### Decoupled RoPE（解耦旋转位置编码）

由于 $c_t^{KV}$ 已被低秩压缩，直接对其施加 RoPE 会破坏矩阵吸收（$W^{UK}$ 无法被吸收进 $W^{UQ}$），导致需要展开后才能计算。

**解决方案：** 为每个头额外增加携带 RoPE 信息的解耦 K/Q 子向量：

$$q_t^R = \text{RoPE}(h_t W^{QR}), \quad k_t^R = \text{RoPE}(h_t W^{KR})$$

最终 Attention 计算：

$$q_t^i = [q_t^{C,i}; q_t^R], \quad k_t^i = [k_t^{C,i}; k_t^R]$$

缓存内容 = $c_t^{KV}$（NoPE 部分）+ $k_t^R$（RoPE 部分），共 $d_c + d_r^h$ 个元素。

#### KV Cache 对比

| 方法 | KV Cache / token / layer |
|---|---|
| MHA | $2 n_h d_h$ |
| GQA | $2 g d_h$ |
| MQA | $2 d_h$ |
| MLA | $d_c + d_r^h$（DeepSeek-V3: $512 + 64 = 576$，而 MHA: $2 \times 128 \times 128 = 32768$） |

**推理优化技巧：** 推理时可将 $W^{UK}$ 吸收进 $W^{UQ}$，避免展开 KV，进一步减少运算量（DeepSeek 官方实现）。

#### 代码骨架

```python
class MLA(nn.Module):
    def __init__(self, d_model, n_heads, d_c, d_rope):
        super().__init__()
        self.W_DKV = nn.Linear(d_model, d_c, bias=False)   # Down-proj KV
        self.W_UK  = nn.Linear(d_c, n_heads * d_head, bias=False)
        self.W_UV  = nn.Linear(d_c, n_heads * d_head, bias=False)
        self.W_DQ  = nn.Linear(d_model, d_c, bias=False)
        self.W_UQ  = nn.Linear(d_c, n_heads * d_head, bias=False)
        self.W_KR  = nn.Linear(d_model, d_rope, bias=False)  # Decoupled RoPE
        self.W_QR  = nn.Linear(d_c, n_heads * d_rope, bias=False)

    def forward(self, h, kv_cache=None):
        c_kv = self.W_DKV(h)                  # 压缩到潜在向量
        k_C  = self.W_UK(c_kv)                # 展开 K（无 RoPE）
        v    = self.W_UV(c_kv)                # 展开 V
        k_R  = apply_rope(self.W_KR(h))       # 解耦 RoPE K
        c_q  = self.W_DQ(h)
        q_C  = self.W_UQ(c_q)
        q_R  = apply_rope(self.W_QR(c_q))
        k = torch.cat([k_C, k_R], dim=-1)     # 完整 K
        q = torch.cat([q_C, q_R], dim=-1)
        # 缓存: c_kv + k_R
        return scaled_dot_product_attention(q, k, v)
```

### 5.5 Flash Attention

IO-aware 精确注意力计算（Dao et al., 2022/2023），通过分块（tiling）计算避免将完整 $n \times n$ 注意力矩阵写入 HBM：

- FlashAttention-2：优化 work partitioning，减少 non-matmul FLOPs，A100 可达 ~72% 理论带宽利用率
- FlashAttention-3：针对 H100 的异步计算，进一步提升到 ~85%
- **意义：** 使长上下文训练（128k+）成为可能，显存从 $O(n^2)$ 降至 $O(n)$

---

## 6. KV Cache 与显存计算

### 6.1 推理阶段两步流程

**Prefill 阶段：** 输入 Prompt 中所有 token 并行计算，生成第一个输出 token，同时将所有 token 的 K/V 缓存至显存。时间复杂度与 Prompt 长度平方相关（$O(n^2d)$）。

**Decode 阶段（自回归生成）：** 每步只生成 1 个新 token，但需读取所有已缓存的 K/V。时间复杂度为 $O(nd)$，受内存带宽限制（memory-bound），而非计算限制。

### 6.2 显存估算公式

假设 BF16/FP16（每参数 2 Bytes），模型层数 $l$，每层 KV 头数 $n_{\text{kv}}$，每头维度 $d_h$，Batch Size $B$，序列长度 $S$：

$$\text{KV Cache 显存} = 2 \times n_{\text{kv}} \times d_h \times l \times B \times S \times 2 \text{ Bytes}$$

（第一个 2 来自 K 和 V，最后一个 2 Bytes 来自 BF16 精度）

$$\text{模型权重显存} \approx N_{\text{params}} \times 2 \text{ Bytes}$$

$$\text{总显存} = \text{KV Cache} + \text{模型权重} + \text{运行时激活值}$$

**Qwen2-72B 示例（$l=80, n_h=64, d_h=128, B=32, S=4096$，GQA $n_{\text{kv}}=8$）：**

$$\text{KV Cache} = 2 \times 8 \times 128 \times 80 \times 32 \times 4096 \times 2 = 40 \text{ GB}$$
$$\text{模型权重} = 72 \times 10^9 \times 2 \text{ Bytes} \approx 144 \text{ GB}$$
$$\text{总计} \approx 184 \text{ GB}$$

> 注：原笔记中 Qwen2-72B 使用了 MHA（$n_{\text{kv}}=64$），若用 GQA（$n_{\text{kv}}=8$），KV Cache 大幅减少。实际 Qwen2-72B 已使用 GQA。

### 6.3 量化对显存的影响

| 精度 | Bytes/参数 | 72B 模型权重 |
|---|---|---|
| FP32 | 4 | ~288 GB |
| BF16/FP16 | 2 | ~144 GB |
| INT8 | 1 | ~72 GB |
| INT4/GPTQ | 0.5 | ~36 GB |
| NF4（QLoRA） | 0.5 | ~36 GB |

---

## 7. FFN → MoE 及路由改进

### 7.1 从 Dense FFN 到 MoE

标准 FFN（两层）：

$$\text{FFN}(x) = \sigma(xW_1 + b_1)W_2 + b_2$$

**MoE（Mixture of Experts）：** 将 FFN 替换为 $N$ 个独立专家（Expert），每个 token 由路由器选择 Top-K 个专家：

$$\text{MoE}(x) = \sum_{i \in \text{Top-K}} g_i(x) \cdot E_i(x)$$

$$g_i(x) = \frac{e^{s_i(x)}}{\sum_{j \in \text{Top-K}} e^{s_j(x)}}, \quad s_i(x) = x \cdot w_i^T$$

**关键性质：** 参数量为 $N \times \text{FFN}_{\text{size}}$，但每次 forward 只激活 $K$ 个专家（$K \ll N$），计算量约等于 $K$ 个 FFN，实现**参数量与计算量解耦**。

### 7.2 DeepSeekMoE：细粒度专家 + 共享专家

DeepSeekMoE（Dai et al., 2024）相比标准 MoE 的改进：

**① 细粒度专家（Fine-grained Expert Segmentation）：**  
将每个原始 FFN 专家拆分为 $m$ 个更小的专家（hidden dim 缩小为 $1/m$），激活专家数从 $K$ 增至 $mK$，总计算量不变。

$$\text{参数量}: N \rightarrow mN, \quad d_{\text{ff}} \rightarrow d_{\text{ff}}/m, \quad \text{激活数}: K \rightarrow mK$$

**效果：** 专家数增加使每个专家更专注于更细粒度的知识，知识分解更充分。

**② 共享专家（Shared Experts）：**  
从 $mN$ 个专家中划出 $K_s$ 个固定激活的共享专家（对所有 token 都激活），其余为路由专家：

$$\text{MoE}(x) = \sum_{i=1}^{K_s} E_i^{\text{shared}}(x) + \sum_{i \in \text{Top-K}_r} g_i(x) E_i^{\text{routed}}(x)$$

**效果：** 共享专家承担通用知识（语法、常识等），释放路由专家专注领域知识，减少专家间冗余。

### 7.3 负载均衡：从辅助损失到无损均衡

#### 传统辅助损失（Switch Transformer / GShard）

$$\mathcal{L}_{\text{aux}} = \alpha \cdot N \sum_{i=1}^N f_i \cdot P_i$$

其中 $f_i$ 是专家 $i$ 处理的 token 比例，$P_i$ 是路由得分均值。**缺点：** 辅助损失梯度与语言建模梯度冲突，需仔细调整 $\alpha$（过大损性能，过小不均衡）。

#### 无辅助损失负载均衡（Loss-Free Balancing，DeepSeek-V3）

核心思想：**引入可动态更新的专家偏置 $b_i$，在 Top-K 选择时使用偏置得分，但门控权重不含偏置**（梯度不受影响）：

$$\text{选择阶段：} \quad \text{Top-K}(s_i(x) + b_i)$$
$$\text{权重计算：} \quad g_i(x) = \frac{e^{s_i(x)}}{\sum_{j \in \text{selected}} e^{s_j(x)}} \quad \text{（无 bias）}$$

偏置更新规则（在梯度外）：

$$b_i \leftarrow b_i - \gamma \cdot \mathbf{1}[\text{expert}_i \text{ overloaded}] + \gamma \cdot \mathbf{1}[\text{expert}_i \text{ underloaded}]$$

**优势：** 完全不引入干扰梯度，模型性能上限更高；同时 DeepSeek-V3 增加了一个极小权重的 sequence-wise 辅助损失作为补充。

**节点限制路由（Node-limited Routing）：** DeepSeek-V3 还限制每个 token 最多被路由到 $M$ 个节点上的专家，减少跨节点通信，提升训练效率。

### 7.4 Mixtral MoE（Mistral AI，2024）

- 8 个专家，每 token 激活 2 个（Top-2），参数量 47B，激活参数约 13B
- 使用辅助损失进行负载均衡
- 在多个 benchmark 上超越 LLaMA-2 70B（激活参数量更小的情况下）

### 7.5 MoE 结构对比

| 模型 | 总参数 | 激活参数 | 专家数/层 | 共享专家 | 路由方式 |
|---|---|---|---|---|---|
| Switch Transformer | 可变 | $\approx 1/N$ | $N$，Top-1 | 无 | 辅助损失 |
| Mixtral 8×7B | 47B | ~13B | 8，Top-2 | 无 | 辅助损失 |
| DeepSeek-V2 | 236B | 21B | 160，Top-6 | 2 | 辅助损失 |
| DeepSeek-V3 | 671B | 37B | 256，Top-8 | 1 | Loss-Free |

---

## 8. 多模态视觉融合：CLIP 及 VLM 架构

### 8.1 CLIP（Contrastive Language-Image Pre-training，OpenAI 2021）

#### 核心思想

用对比学习在**4亿图文对**上训练，使同一对的图像和文本 embedding 相近，不同对的相远。

#### 训练目标

对 $N$ 个图文对，计算 $N \times N$ 相似度矩阵，对角线为正样本：

$$\mathcal{L}_{\text{CLIP}} = \frac{1}{2N}\sum_{i=1}^N \left[-\log\frac{e^{\text{sim}(I_i, T_i)/\tau}}{\sum_j e^{\text{sim}(I_i, T_j)/\tau}} - \log\frac{e^{\text{sim}(T_i, I_i)/\tau}}{\sum_j e^{\text{sim}(T_j, I_i)/\tau}}\right]$$

$\tau$ 为可学习温度参数（初始 0.07），关键超参之一。

#### 架构

- **图像编码器：** ViT-B/32, ViT-L/14 等；输出 $[CLS]$ token embedding
- **文本编码器：** Causal Transformer（类 GPT 结构）；输出 EOS token embedding
- **对比投影：** 线性投影至相同维度（512 / 768）

#### CLIP 局限性

- 缺乏精细粒度的视觉理解（dense prediction 能力弱）
- 对多目标、空间关系的理解不足
- 零样本泛化受限于训练数据分布

### 8.2 CLIP 改进系列

| 方法 | 改进点 |
|---|---|
| **ALIGN** (Google, 2021) | 更大规模噪声数据（18亿对），证明数据量比质量更重要 |
| **SigLIP** (Zhai et al., 2023) | 将 Softmax-CE 替换为 Sigmoid 逐对损失，支持更大 batch，训练更稳定 |
| **CoCa** (Yu et al., 2022) | 图像编码器 + 文本解码器，对比损失 + 生成损失双目标 |
| **EVA-CLIP** (BGE/BAAI) | 更大 ViT 参数（18B），在 OpenCLIP 上训练，作为多模态主干 |
| **DFN** (Apple, 2023) | 数据过滤网络，用小模型过滤网络爬取数据，提升数据质量 |

#### SigLIP 损失

替换 softmax 归一化，每对独立计算 sigmoid 交叉熵：

$$\mathcal{L}_{\text{SigLIP}} = -\frac{1}{N}\sum_{i,j} y_{ij} \log\sigma(z_{ij}/\tau - b) + (1-y_{ij})\log(1-\sigma(z_{ij}/\tau - b))$$

其中 $y_{ij}=1$ 当且仅当 $i=j$，$z_{ij} = \text{sim}(I_i, T_j)$。**优点：** 不依赖全局 Softmax，无需跨设备 gather，大规模分布式训练更高效。

### 8.3 VLM 主流架构

#### 架构一：Visual Token Fusion（主流方案）

将图像通过 Vision Encoder 得到视觉 tokens，经**投影层（Projector）** 映射到 LLM 的 token 空间，与文本 tokens 拼接后送入 LLM：

```
Image → ViT (Vision Encoder) → [Patch Tokens] → MLP Projector → LLM Token Space
Text  ─────────────────────────────────────────────────────────→ LLM
```

代表模型：LLaVA 系列（1.0/1.5/Next），Qwen2-VL，InternVL，MiniCPM-V。

**Projector 设计：**
- 简单 MLP（LLaVA-1.5 使用两层 MLP，效果超过交叉注意力）
- MLP + Token Compression（如 LLaVA-NeXT 对高分辨率图像分块压缩）
- Resampler/Q-Former（固定数量 query tokens，见下）

#### 架构二：Q-Former（BLIP-2，InstructBLIP）

引入可学习的 **Query Transformer**，用 $M$ 个可学习 query 向量从 ViT 特征中提炼固定数量的视觉表征：

$$Q_{\text{out}} = \text{CrossAttn}(Q_{\text{learnable}}, V_{\text{img}}) + \text{SelfAttn}(Q_{\text{learnable}})$$

**优点：** 无论图像分辨率如何，输出 token 数固定（如 32），减少 LLM 计算量。  
**缺点：** 信息压缩可能丢失细节；Q-Former 的两阶段预训练复杂。

#### 架构三：Cross-Attention（Flamingo，DeepMind 2022）

在 LLM 的每隔若干层插入交叉注意力层，图像 tokens 作为 Key/Value，LLM 文本 tokens 作为 Query：

$$\text{GatedXAttn}(x) = x + \tanh(\alpha) \cdot \text{CrossAttn}(x, V_{\text{img}})$$

**优点：** 视觉信息在整个解码过程中持续注入。  
**缺点：** 计算量增加，结构修改较大。

### 8.4 Qwen2-VL 高分辨率处理（Naive Dynamic Resolution）

将图像分割为不同数量的 patch（不 padding/resize 到固定分辨率），使用 M-RoPE 为每个 patch 分配二维位置编码，LLM 直接处理变长视觉序列。

**2D RoPE for ViT：**  
对 ViT 内部的 Attention，也用二维 RoPE（行位置 $r$，列位置 $c$）替代原始 1D 或绝对位置编码。

---

## 9. 大模型训练与推理框架

### 9.1 分布式预训练框架

#### 并行策略

**数据并行（Data Parallelism, DP）：** 每个 GPU 持有完整模型副本，处理不同批次数据，梯度同步（AllReduce）。适合中小模型。

**张量并行（Tensor Parallelism, TP）：** 将权重矩阵按列/行切分到不同 GPU（Megatron-LM，Narayanan et al. 2021）。典型 TP-8。通信原语：AllReduce / AllGather。

**流水线并行（Pipeline Parallelism, PP）：** 将模型层分配到不同 GPU，使用 micro-batch 填充流水线气泡（1F1B 调度，Megatron-LM；Zero-bubble PP）。

**序列并行（Sequence Parallelism, SP）：** 沿序列维度切分，与 TP 结合处理超长序列（Ring-Attention，FlashAttention 支持）。

**专家并行（Expert Parallelism, EP）：** MoE 场景，将不同专家分配到不同 GPU，All-to-All 通信。

**3D/4D 并行：** DP + TP + PP（+ SP/EP）组合，DeepSeek-V3 训练采用 16 路 EP + 8 路 TP + 64 路 PP 组合。

#### Megatron-LM

NVIDIA 出品，工业级 LLM 预训练框架，支持 TP/PP/SP 及其组合，优化 all-reduce 通信，与 DeepSpeed 可集成（Megatron-DeepSpeed）。  
关键特性：fused kernel、activation checkpointing、FP8 训练支持。

#### DeepSpeed（ZeRO 系列）

微软开源，通过 **ZeRO（Zero Redundancy Optimizer）** 消除数据并行中的显存冗余：

| ZeRO Stage | 分片内容 | 显存节省 |
|---|---|---|
| Stage 1 | Optimizer States | $\sim 4\times$ |
| Stage 2 | Optimizer States + Gradients | $\sim 8\times$ |
| Stage 3 | Optimizer States + Gradients + Parameters | $\sim N\times$（N 为 DP 数） |

**ZeRO-Offload：** 将 Optimizer States 卸载到 CPU 内存，进一步节省 GPU 显存。  
**ZeRO-Infinity：** 扩展至 NVMe 存储，理论上支持无限规模模型。

### 9.2 后训练框架

#### LLaMA-Factory

全栈 LLM 微调框架（[文档](https://llamafactory.readthedocs.io/zh-cn/latest/)），支持 SFT/RLHF/GRPO，几乎兼容所有主流模型，提供统一配置接口：

```yaml
# 典型 SFT 配置
model_name_or_path: meta-llama/Llama-3-8B
stage: sft
do_train: true
finetuning_type: lora
lora_rank: 64
dataset: alpaca_en
```

支持：LoRA/QLoRA/Full SFT，DPO/ORPO/SimPO，GRPO。

#### VeRL（Volcano Engine Reinforcement Learning）

字节跳动 Volcano Engine 开源，专为 LLM RLHF 设计，特点：
- 将 actor/critic/reward 模型在同一框架中管理，支持 FSDP + Megatron-LM 混合并行
- 支持异步 rollout（actor 生成和 critic 评分并行进行）
- 与 vLLM 集成加速推理阶段

#### AReal（Alibaba）

阿里 Qwen 团队的 RL 后训练框架，强调**在线强化学习**效率，通过异步架构使 policy model 训练与 rollout 解耦，GPU 利用率更高。

#### SLIME（MSRA）

微软研究院的 Scalable LLM Instruction tuning with Multi-Expert system，专注于大规模指令微调与 RLHF。

#### Unsloth

轻量化高效 LoRA 微调库，手写 Triton kernel 优化 LoRA 相关计算，相比 HuggingFace PEFT 可节省 30-50% 显存，并提速 2-5 倍。

### 9.3 推理框架

#### vLLM

加州大学伯克利分校开源，LLM 推理服务框架，核心创新为 **PagedAttention**：

**PagedAttention 原理：** 将 KV Cache 划分为固定大小的物理块（Block），维护逻辑块到物理块的映射表。序列不同、长度动态变化，但物理块固定分配，避免内存碎片，GPU 内存利用率提升至 ~90%（对比 HuggingFace naive 实现 ~60%）。

**Copy-on-Write（CoW）：** 不同请求共享相同 prefix 的物理块（Prefix Caching），减少重复计算。

**Chunked Prefill：** 将长 prefill 分块执行，与 decode 交错，降低 Time-to-First-Token（TTFT）。

```python
from vllm import LLM, SamplingParams
llm = LLM(model="meta-llama/Llama-3-8B", tensor_parallel_size=4)
outputs = llm.generate(["Hello, my name is"], SamplingParams(max_tokens=100))
```

#### SGLang

斯坦福/UC Berkeley 开源，更聚焦**结构化生成**和**高吞吐推理**：

- **RadixAttention：** 将 Prefix Cache 组织为 Radix Tree，自动复用前缀 KV Cache，适合有大量公共前缀的批量推理
- **Compressed State Machine：** 对 JSON/正则等结构化输出，通过约束解码 + 状态机压缩提升速度
- **连续批处理（Continuous Batching）：** 动态添加/删除序列，最大化 GPU 利用率

**vLLM vs SGLang 对比：**

| 特性 | vLLM | SGLang |
|---|---|---|
| KV Cache 管理 | PagedAttention | RadixAttention + Paging |
| 结构化生成 | 基础支持 | 深度优化 |
| 多模态支持 | 完整 | 完整 |
| 生态成熟度 | 更成熟，用户多 | 后来居上，部分场景更快 |
| TP 支持 | 完整 | 完整 |

### 9.4 应用开发框架：LangChain + LangGraph

#### LangChain

LLM 应用开发框架，提供**链式调用（Chain）**、**工具集成（Tools）**、**文档加载（Document Loaders）**、**向量存储（VectorStore）** 等抽象。

核心概念：
- `Runnable`：统一接口，支持 `invoke / stream / batch`
- `Chain`：`LLMChain`, `ConversationChain`, `RetrievalQA` 等预构建链
- `Agent`：基于 LLM 进行工具选择和调用的自主执行单元

#### LangGraph

基于 LangChain 的**有状态多角色 Agent 工作流框架**，用图（Graph）描述 Agent 的状态机：

```python
from langgraph.graph import StateGraph, END

workflow = StateGraph(AgentState)
workflow.add_node("agent", run_agent)
workflow.add_node("tools", run_tools)
workflow.add_edge("agent", "tools")         # 条件边：agent 决定调用工具
workflow.add_edge("tools", "agent")         # 工具结果返回 agent
workflow.add_conditional_edges("agent", should_continue, {"yes": "tools", "no": END})
```

**适用场景：** 多步骤推理、Human-in-the-loop、并行工具调用、多 Agent 协作。

---

## 10. 模型后训练：SFT · LoRA · RLHF · GRPO

### 10.1 监督微调（SFT）

在指令-响应对 $\{(x_i, y_i)\}$ 上最大化条件语言模型似然：

$$\mathcal{L}_{\text{SFT}} = -\sum_{i} \sum_{t=1}^{|y_i|} \log P_\theta(y_{i,t} | x_i, y_{i,<t})$$

**注意：** 通常只对响应部分计算损失（`labels` 中 prompt 位置设为 -100）。

**数据质量 >> 数据数量：** 少量高质量指令数据（1k-10k）通常优于大量低质量数据。

### 10.2 参数高效微调（PEFT）

#### LoRA（Low-Rank Adaptation，Hu et al. 2021）

冻结原始权重 $W_0 \in \mathbb{R}^{d \times k}$，只训练低秩分解矩阵 $A \in \mathbb{R}^{d \times r}$ 和 $B \in \mathbb{R}^{r \times k}$（$r \ll \min(d,k)$）：

$$h = W_0 x + \Delta W x = W_0 x + BAx$$

初始化：$B$ 全零，$A$ 随机高斯，使 $\Delta W$ 初始为零。  
**可训练参数比例：** $r(d+k)/(dk) \approx r/\min(d,k)$，$r=8$ 时约 0.1%-0.5%。

**融合推理：** $W = W_0 + BA$（将 LoRA 权重合并回原权重，推理无额外开销）。

```python
# 使用 PEFT 库
from peft import LoraConfig, get_peft_model
config = LoraConfig(r=64, lora_alpha=128, target_modules=["q_proj", "v_proj"],
                    lora_dropout=0.05, bias="none")
model = get_peft_model(base_model, config)
```

#### QLoRA（Dettmers et al. 2023）

将 base model 量化为 **NF4（4-bit Normal Float）** 保存，LoRA adapter 在 BF16 上训练：

- NF4 量化：基于正态分布设计的 4-bit 数值格式，相比 INT4 更适合权重分布
- **双量化（Double Quantization）：** 对量化常数也做二次量化，进一步节省显存
- **分页优化器（Paged Optimizer）：** 使用 NVIDIA Unified Memory 自动在 GPU/CPU 间分页，应对梯度检查点的显存峰值

65B 模型可在单张 48GB A100 上全参量级别地微调，但实际更新的只有 LoRA adapter 参数。

#### LoRA 变体

| 变体 | 改进点 |
|---|---|
| AdaLoRA | 根据参数重要性自适应分配秩 $r$ |
| DoRA | 将权重分解为方向（Direction）+ 幅度（Magnitude），分别更新 |
| LoRA+ | A/B 使用不同学习率（$\eta_B = \lambda\eta_A$），收敛更快 |
| VeRA | 跨层共享 A/B，只学习向量缩放，参数量更小 |
| MoLoRA | 多 LoRA 专家混合，不同任务激活不同 adapter |

### 10.3 PPO（Proximal Policy Optimization）

PPO 是 RLHF 中 Actor-Critic 框架的基础算法（Schulman et al. 2017）。

**目标函数：**

$$\mathcal{L}_{\text{PPO}}(\theta) = \mathbb{E}_t \left[\min\left(r_t(\theta) \hat{A}_t, \text{clip}(r_t(\theta), 1-\epsilon, 1+\epsilon)\hat{A}_t\right)\right]$$

其中 $r_t(\theta) = \frac{\pi_\theta(a_t|s_t)}{\pi_{\theta_{\text{old}}}(a_t|s_t)}$ 为重要性权重，$\hat{A}_t$ 为优势函数估计，$\epsilon$ 为裁剪超参（通常 0.1-0.2）。

**RLHF-PPO 四模型架构：**
1. **Policy Model（Actor）：** 待训练的 LLM
2. **Reward Model（RM）：** 由人类偏好数据训练的奖励预测模型
3. **Critic（Value Model）：** 估计当前状态价值，通常与 Policy 同规模
4. **Reference Model（SFT Model）：** 冻结的 SFT 模型，用于计算 KL 惩罚

**总奖励（含 KL 惩罚）：**

$$r(x, y) = r_\phi(x, y) - \beta \cdot \text{KL}[\pi_\theta(\cdot|x) \| \pi_{\text{ref}}(\cdot|x)]$$

**PPO 的问题：** 四个模型并发，显存压力大；超参敏感；训练不稳定；奖励模型可能被 hack（reward hacking）。

### 10.4 DPO（Direct Preference Optimization，Rafailov et al. 2023）

将 RLHF 目标直接优化为对偏好数据的最大似然，绕过显式 RM 训练：

$$\mathcal{L}_{\text{DPO}}(\theta) = -\mathbb{E}_{(x, y_w, y_l)} \left[\log\sigma\left(\beta \log\frac{\pi_\theta(y_w|x)}{\pi_{\text{ref}}(y_w|x)} - \beta \log\frac{\pi_\theta(y_l|x)}{\pi_{\text{ref}}(y_l|x)}\right)\right]$$

其中 $y_w$ 为 win 响应，$y_l$ 为 lose 响应，$\beta$ 为 KL 约束强度。

**DPO 本质：** 隐式奖励 $r(x,y) = \beta\log\frac{\pi_\theta(y|x)}{\pi_{\text{ref}}(y|x)}$，DPO 直接优化这一隐式奖励上的偏好分类损失。

**优点：** 无需 Reward Model 和 Critic，稳定易训。  
**缺点：** 离线算法，无法动态探索；对 out-of-distribution 偏好数据泛化较差。

#### DPO 变体

| 方法 | 特点 |
|---|---|
| ORPO | 将偏好学习整合进 SFT 阶段，无需单独 RM，单步训练 |
| SimPO | 去除 reference model，用平均 log-likelihood 替代，更简洁高效 |
| KTO | 只需 binary 好/坏标签（无需偏好对），通过 Kahneman-Tversky 效用理论建模 |

### 10.5 GRPO（Group Relative Policy Optimization，DeepSeek 2024）

**来源：** 首次在 DeepSeekMath（Shao et al., 2024，[arXiv 2402.03300](https://arxiv.org/abs/2402.03300)）中提出，后用于 DeepSeek-R1 训练。

#### 核心思想

去除 PPO 中的 Critic（Value Model），改用**组内归一化**估计基线（Baseline），从而将四模型架构简化为两模型（Policy + Reference）：

1. 对每个 prompt $q$，用当前 policy $\pi_{\theta_{\text{old}}}$ 采样 $G$ 个响应 $\{o_i\}_{i=1}^G$
2. 用奖励函数 $r_i$ 对每个响应评分
3. 以组内均值为基线计算优势：

$$\hat{A}_i = \frac{r_i - \text{mean}(\{r_1,\ldots,r_G\})}{\text{std}(\{r_1,\ldots,r_G\})}$$

4. 用 PPO-clip 形式更新 policy：

$$\mathcal{L}_{\text{GRPO}}(\theta) = -\frac{1}{G}\sum_{i=1}^G\frac{1}{|o_i|}\sum_{t=1}^{|o_i|}\left[\min\!\left(r_{i,t}(\theta)\hat{A}_i, \text{clip}(r_{i,t}(\theta), 1-\epsilon, 1+\epsilon)\hat{A}_i\right) - \beta\,\text{KL}_t\right]$$

其中：
$$r_{i,t}(\theta) = \frac{\pi_\theta(o_{i,t}|q, o_{i,<t})}{\pi_{\theta_{\text{old}}}(o_{i,t}|q, o_{i,<t})}, \quad \text{KL}_t = \log\frac{\pi_\theta(o_{i,t}|q, o_{i,<t})}{\pi_{\text{ref}}(o_{i,t}|q, o_{i,<t})}$$

**关键对比 PPO：**

| 特性 | PPO | GRPO |
|---|---|---|
| Critic 模型 | 需要（同规模 Value Model） | **不需要** |
| 显存占用 | 4 个模型 | 2 个模型 |
| 基线估计 | Value Network 预测 | 组内平均奖励 |
| 稳定性 | 较差，超参敏感 | 较好 |

#### 奖励函数设计（DeepSeek-R1）

**可验证奖励（Verifiable Rewards，RLVR）：**

- **准确性奖励（Accuracy Reward）：** 对数学问题，规则验证最终答案是否正确（如 BoxAnswer 格式匹配）；对代码，用 OJ 系统编译运行
- **格式奖励（Format Reward）：** 强制要求输出包含 `<think>...</think>` + 最终答案的特定格式
- **不使用神经网络 Reward Model：** 避免奖励 hacking（model 找到欺骗 RM 的方式）

**奖励 Hacking 防御：** 规则奖励天然抗 hacking，因为无法通过优化欺骗规则检查器。

#### GRPO 改进与扩展

**DAPO（DAPO: An Open-Source LLM Reinforcement Learning System，字节跳动 2025）：**
- 移除 KL 惩罚项（$\beta=0$），避免限制探索
- 改用 clip-higher 策略（只 clip ratio < $1-\epsilon$，不 clip ratio > $1+\epsilon$），鼓励策略更新
- 动态采样：过滤全对/全错的 group（无梯度信号），只保留混合结果组
- 在 AIME 2024 上达到 50 分（DeepSeek-R1 为 79.8 分）

**Dr. GRPO（2025）：**  
针对 GRPO 的偏差（length bias：更长输出倾向于获得更高奖励）提出修正，在归一化中控制长度因素。

**GRPO for Agentic AI（Agent-GRPO）：**
将 GRPO 扩展到多步 Agent 场景：
- reward 来自**工具调用结果**（代码执行结果、搜索结果正确性）
- 每个 rollout 包含多轮工具调用，使用**过程奖励（Process Reward）** 或只用最终结果（Outcome Reward）
- 典型框架：VeRL 的多轮对话支持，AReal 的异步 rollout

### 10.6 RLHF 全流程

```
预训练 LLM
    ↓ SFT（指令微调）
SFT Model
    ↓ 人类标注偏好数据（Preference Data）
    ↓ 训练 Reward Model（Bradley-Terry 模型）
    ↓ PPO / GRPO 强化学习
RLHF-aligned Model
```

**Bradley-Terry 偏好模型：**

$$P(y_w \succ y_l | x) = \sigma(r(x, y_w) - r(x, y_l))$$

**RM 训练损失：**

$$\mathcal{L}_{\text{RM}} = -\mathbb{E}_{(x, y_w, y_l)}[\log\sigma(r_\phi(x, y_w) - r_\phi(x, y_l))]$$

---

## 11. 模型蒸馏与量化

### 11.1 知识蒸馏（Knowledge Distillation，Hinton et al. 2015）

用**教师模型（Teacher）** 的软标签（soft logits）训练**学生模型（Student）**：

$$\mathcal{L}_{\text{KD}} = (1-\alpha)\mathcal{L}_{\text{CE}}(y, \hat{p}) + \alpha T^2 \mathcal{L}_{\text{KL}}(p_T, p_S)$$

其中 $T$ 为温度（控制软标签平滑度），$p_T = \text{softmax}(z_T/T)$，$p_S = \text{softmax}(z_S/T)$。

**对 LLM 的意义：** 软标签包含教师对语义相似 token 的隐式知识（如"猫"和"猫咪"的相对概率），比 one-hot 标签信息更丰富。

#### LLM 蒸馏策略

**① 黑盒蒸馏（数据蒸馏）：** 用强大教师模型生成高质量数据（Alpaca、Orca、WizardLM），直接 SFT 学生模型。GPT-4 → LLaMA-7B 是典型应用。

**② 白盒蒸馏（特征对齐）：** 对齐中间层表征，如 attention map、hidden states：

$$\mathcal{L}_{\text{feat}} = \sum_l \|h_S^l - \text{Proj}(h_T^l)\|_2^2$$

**③ DeepSeek-R1 蒸馏方案：**  
用 R1 模型（671B MoE）生成带有 `<think>` 推理过程的数据，然后 SFT 蒸馏到 LLaMA/Qwen 等较小 dense 模型上。DeepSeek-R1-Distill-Qwen-32B 在数学推理上超过大多数 70B 模型。

### 11.2 后训练量化（PTQ）

#### GPTQ（Frantar et al. 2022）

基于二阶 Hessian 信息的逐层 INT4 量化，利用 OBQ（Optimal Brain Quantization）框架：

$$\delta W_q = -\frac{w_q}{[H^{-1}]_{qq}} \cdot (H^{-1})_{:,q}$$

逐列量化，每量化一列后补偿其余列，使量化误差最小化。支持 4/3/2bit，在 175B GPT 上仅增加约 1% 困惑度。

#### AWQ（Activation-aware Weight Quantization，Lin et al. 2023）

观察到：权重的重要性由**激活大小**决定（少量权重对应大激活值，对输出影响大）。  
对这些"重要权重"做缩放（scale），保持其量化精度：

$$W_q = \text{quant}(W \cdot s) / s, \quad s \propto |W|^{0.5} \cdot \max(|X|)^{0.5}$$

比 GPTQ 更快（无需 Hessian 逆），LLM 量化的主流方法之一。

#### GGUF / llama.cpp Q4_K_M 等

社区量化格式，支持 2-8bit 量化，可在 CPU 上推理（通过 llama.cpp）。K-Quant 对不同层使用不同 bit（注意力层更高精度）。

### 11.3 量化感知训练（QAT）

在训练过程中模拟量化误差（Straight-Through Estimator, STE），使模型对量化更鲁棒：

$$\hat{w} = \text{quant}(w), \quad \frac{\partial \mathcal{L}}{\partial w} = \frac{\partial \mathcal{L}}{\partial \hat{w}} \cdot \mathbf{1}[|w| \leq \Delta]$$

**LLM.int8()（bitsandbytes）：** 将大激活值（离群值）在 FP16 中处理，其余用 INT8，避免量化误差集中在离群激活上。

### 11.4 KV Cache 量化

KV Cache 本身也可量化（KIVI、KVQuant 等），将 K/V 量化为 INT4/INT2 存储，推理时反量化：

- **KV-INT4：** 基本无精度损失，KV Cache 显存减半
- **KV-INT2：** 显存减至 1/4，轻微精度损失
- MLA 的压缩本身可视为一种隐式低精度表示

### 11.5 蒸馏 + 量化 + GRPO 协同

```
大模型（GPT-4o / DeepSeek-R1）
    ↓ 数据蒸馏：生成带 CoT 的高质量数据
中等模型 SFT（如 Qwen2.5-7B）
    ↓ GRPO / DPO 进一步对齐
对齐后模型
    ↓ AWQ/GPTQ INT4 量化
部署就绪模型（边缘/API服务）
```

---

## 12. Agentic AI：训练框架与推理框架

### 12.1 Agentic AI 基础概念

**Agent 定义：** 能够感知环境、执行工具调用、进行多步推理和规划、并根据反馈迭代改进的 LLM 驱动系统。

**核心能力：**
- **工具使用（Tool Use）：** 函数调用（Function Calling）、代码执行、搜索
- **多步规划（Multi-step Planning）：** CoT、ReAct（Reason+Act）、Tree of Thought
- **记忆（Memory）：** 短期（上下文窗口）+ 长期（向量数据库）
- **自我反思（Self-reflection）：** Reflexion，CRITIC 等

### 12.2 ReAct 框架

LLM 交替进行 Reason（生成思考）和 Act（工具调用），形成 Thought-Action-Observation 循环：

```
Thought: 我需要搜索当前北京的天气
Action: search("北京今天天气")
Observation: 北京今天晴，25°C
Thought: 已获取天气信息，可以回答用户
Answer: 北京今天晴天，温度25°C
```

### 12.3 Agent 微调（Agent-Tuning）

#### 训练数据构造

使用 Self-Instruct 或更强大的教师模型生成 Agent 轨迹数据：
- **轨迹格式：** `<query> → <thought_1> → <action_1> → <obs_1> → ... → <final_answer>`
- **工具调用格式：** Function Calling Schema（OpenAI 格式）或 XML 格式
- **数据来源：** WebShop、GAIA、SWE-bench（代码 Agent）、MINT 等 benchmark

#### Fireact / AgentTuning

清华大学工作，收集多任务 Agent 轨迹（基于 GPT-4），微调 LLaMA 获得较强 Agent 能力。

### 12.4 GRPO for Agentic AI

将 GRPO 应用于多步 Agent 任务：

**关键修改：**
1. **轨迹级 Reward：** 对完整的工具调用序列计算最终奖励（任务完成度、工具调用正确性）
2. **过程奖励模型（PRM）：** 对中间步骤（每次工具调用结果）也给予奖励，比纯结果奖励梯度信号更密
3. **多步 Token-level Advantage：** GRPO 优势函数从单步扩展到多轮：

$$\hat{A}_i = \frac{R_i - \text{mean}(\{R_j\}_{j=1}^G)}{\text{std}(\{R_j\}_{j=1}^G)}$$

其中 $R_i$ 为第 $i$ 条轨迹的总奖励（可包含中间步骤奖励折扣）。

4. **Rollout 并行化：** 用 vLLM / SGLang 加速 rollout（Agent 轨迹生成），VeRL/AReal 支持异步 Actor。

#### Kimi-k1.5 / Kimi-k2

Moonshot 团队将 Agentic RL 扩展到长上下文多步推理，在 code + tool-use 任务上取得大幅提升，采用在线 RL + 工具调用奖励。

#### OpenHands / SWE-agent

针对代码 Agent 的推理框架，支持 LLM 在 sandbox 环境中执行代码、修改文件、运行测试。适合 SFT + GRPO 训练 coding agent。

### 12.5 推理框架对 Agentic AI 的支持

**SGLang 的关键特性：**
- **RadixAttention Prefix Cache：** Agent 多轮对话中系统 prompt 和工具定义为公共前缀，自动缓存
- **Structured Output：** 强制工具调用输出符合 JSON Schema，避免格式错误
- **Speculative Execution：** 预测性执行下一步工具调用

**vLLM 的关键特性：**
- **Multi-Step Scheduling：** 支持 Agent 轨迹中的多步 token 生成，批次不同步的序列
- **Tool-calling API 兼容：** 与 OpenAI Function Calling 接口兼容，便于接入 LangGraph

---

## 13. 重点论文与项目索引

### 13.1 基础架构

| 论文 | 贡献 |
|---|---|
| [Attention Is All You Need](https://arxiv.org/abs/1706.03762)（2017） | Transformer 架构 |
| [An Image is Worth 16x16 Words](https://arxiv.org/abs/2010.11929)（2020） | ViT |
| [RoFormer: Enhanced Transformer with Rotary Position Embedding](https://arxiv.org/abs/2104.09864)（2021） | RoPE |
| [GQA: Training Generalized Multi-Query Transformer Models](https://arxiv.org/abs/2305.13245)（2023） | GQA |
| [DeepSeek-V2](https://arxiv.org/abs/2405.04434)（2024） | MLA, DeepSeekMoE |
| [DeepSeek-V3 Technical Report](https://arxiv.org/abs/2412.19437)（2024） | Loss-Free Balancing, MTP |
| [FlashAttention-2](https://arxiv.org/abs/2307.08691)（2023） | IO-aware 高效 Attention |

### 13.2 多模态

| 论文 | 贡献 |
|---|---|
| [CLIP](https://arxiv.org/abs/2103.00020)（OpenAI, 2021） | 视觉-语言对比预训练 |
| [SigLIP](https://arxiv.org/abs/2303.15343)（2023） | Sigmoid 对比损失 |
| [BLIP-2](https://arxiv.org/abs/2301.12597)（2023） | Q-Former, 冻结 LLM |
| [LLaVA](https://arxiv.org/abs/2304.08485)（2023） | 简单 MLP Projector + SFT |
| [LLaVA-1.5](https://arxiv.org/abs/2310.03744)（2023） | 简单有效，MLP 比交叉注意力更好 |
| [InternVL2](https://arxiv.org/abs/2312.14238)（2024） | 大 ViT + LLM，开源最强之一 |
| [Qwen2-VL](https://arxiv.org/abs/2409.12191)（2024） | M-RoPE, 动态分辨率 |
| [Flamingo](https://arxiv.org/abs/2204.14198)（DeepMind, 2022） | 交叉注意力融合，few-shot VLM |

### 13.3 对齐与后训练

| 论文 | 贡献 |
|---|---|
| [InstructGPT / RLHF](https://arxiv.org/abs/2203.02155)（OpenAI, 2022） | 人类反馈 RL，PPO 对齐 |
| [LoRA](https://arxiv.org/abs/2106.09685)（2021） | 低秩适配微调 |
| [QLoRA](https://arxiv.org/abs/2305.14314)（2023） | 4-bit 量化 + LoRA |
| [DPO](https://arxiv.org/abs/2305.18290)（2023） | 直接偏好优化，无需 RM |
| [DeepSeekMath / GRPO](https://arxiv.org/abs/2402.03300)（2024） | Group Relative Policy Optimization |
| [DeepSeek-R1](https://arxiv.org/abs/2501.12948)（2025） | RLVR, 推理涌现，R1-Zero |
| [DAPO](https://arxiv.org/abs/2503.14476)（字节跳动, 2025） | GRPO 改进：clip-higher, 动态采样 |

### 13.4 推理与部署

| 论文/项目 | 贡献 |
|---|---|
| [vLLM](https://arxiv.org/abs/2309.06180)（2023） | PagedAttention, 高吞吐 LLM 推理 |
| [SGLang](https://arxiv.org/abs/2312.07104)（2023） | RadixAttention, 结构化生成 |
| [AWQ](https://arxiv.org/abs/2306.00978)（2023） | 激活感知权重量化 |
| [GPTQ](https://arxiv.org/abs/2210.17323)（2022） | 逐层 INT4 量化 |

### 13.5 分布式训练

| 论文/项目 | 贡献 |
|---|---|
| [Megatron-LM](https://arxiv.org/abs/1909.08053)（NVIDIA） | 张量并行 + 流水线并行 |
| [ZeRO / DeepSpeed](https://arxiv.org/abs/1910.02054)（微软） | ZeRO 显存优化 |
| [Ring Attention](https://arxiv.org/abs/2310.01889)（2023） | 序列并行，超长上下文 |

### 13.6 Agentic AI

| 论文/项目 | 贡献 |
|---|---|
| [ReAct](https://arxiv.org/abs/2210.03629)（2022） | Reason + Act 框架 |
| [Toolformer](https://arxiv.org/abs/2302.04761)（Meta, 2023） | 自监督工具调用学习 |
| [AgentTuning](https://arxiv.org/abs/2310.12823)（清华, 2023） | 多任务 Agent SFT |
| [OpenHands](https://github.com/All-Hands-AI/OpenHands) | 代码 Agent 推理框架 |
| [LangGraph](https://github.com/langchain-ai/langgraph) | 有状态多 Agent 工作流 |
| [VeRL](https://github.com/volcengine/verl) | LLM RLHF 训练框架 |

---

> **说明：** 本笔记综合了 Transformer / ViT 论文原文、DeepSeek-V2/V3 技术报告、DeepSeekMath / R1 论文、主流框架官方文档及社区高质量解析文章。公式以数学推导为主，建议结合原论文对照阅读。
