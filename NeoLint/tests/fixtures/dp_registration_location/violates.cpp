namespace NeoWPF::UI::Properties
{
template<typename TOwner, typename TProperty = int>
void RegisterProperty(const char*, TProperty = {}) {}
}

void RegisterButtonPropertyInControlFile()
{
    NeoWPF::UI::Properties::RegisterProperty<int>("Text");
}
