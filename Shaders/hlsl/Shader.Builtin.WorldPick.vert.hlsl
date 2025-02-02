struct UBO
{
    float4x4 proj;
    float4x4 view;
    float4 ambient_color;
    float3 view_position;
};

struct PushConstant
{
    float4x4 model;
};

struct VSInput
{
    [[vk::location(0)]] float3 vPosition : POSITION0;
};

[[vk::binding(0, 0)]] ConstantBuffer<UBO> ubo;
[[vk::push_constant]] ConstantBuffer<PushConstant> push_constants;

float4 main(VSInput input) : SV_POSITION
{
    return mul(ubo.proj, mul(ubo.view, mul(push_constants.model, float4(input.vPosition, 1.0f))));
}