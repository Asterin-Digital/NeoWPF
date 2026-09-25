enum class PropertyValueSource
{
    Local,
    Style,
    Trigger,
};

class UIElement
{
public:
    void SetNamedPropertyValue(int, int, PropertyValueSource);

    void SetTriggerPropertyValue(int property, int value)
    {
        // neolint-disable-next-line neowpf-property-source-tag: legacy source behavior
        SetNamedPropertyValue(property, value, PropertyValueSource::Local);
    }
};
