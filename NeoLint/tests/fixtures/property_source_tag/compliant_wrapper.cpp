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
        SetNamedPropertyValue(property, value, PropertyValueSource::Trigger);
    }
};
