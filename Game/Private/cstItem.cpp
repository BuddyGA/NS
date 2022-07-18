#include "cstItem.h"



NS_CLASS_BEGIN(cstItem, nsObject)
NS_CLASS_END(cstItem)

cstItem::cstItem()
{
	SpawnActorClass = nullptr;
	Category = cstEItemCategory::CONSUMABLE;
}




NS_CLASS_BEGIN(cstEquipment, nsActor)
NS_CLASS_END(cstEquipment)

cstEquipment::cstEquipment()
{
}




NS_CLASS_BEGIN(cstWeapon, cstEquipment)
NS_CLASS_END(cstWeapon)

cstWeapon::cstWeapon()
{

}




NS_CLASS_BEGIN(cstArmor, cstEquipment)
NS_CLASS_END(cstArmor)

cstArmor::cstArmor()
{

}
