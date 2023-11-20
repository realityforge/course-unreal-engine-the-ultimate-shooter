/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TextureSubType.h"

const FName FTextureSubTypeUtil::MetaDataKey{ TEXT("TextureSubTypes") };

TArray<ETextureSubType> FTextureSubTypeUtil::ExtractFromMetaData(const UObject* Object)
{
    TArray<ETextureSubType> SubTypes;
    if (IsValid(Object))
    {
        const UEnum* Enum = StaticEnum<ETextureSubType>();
        const FString Value = Object->GetPackage()->GetMetaData()->GetValue(Object, MetaDataKey);
        TArray<FString> Out;
        Value.ParseIntoArray(Out, TEXT(","), true);
        for (auto& Part : Out)
        {
            if (const int64 EnumValue = Enum->GetValueByNameString(Part); INDEX_NONE == EnumValue)
            {
                SubTypes.Reset();
                return SubTypes;
            }
            else
            {
                SubTypes.Add(static_cast<ETextureSubType>(EnumValue));
            }
        }
    }
    return SubTypes;
}

bool FTextureSubTypeUtil::DoesMetaDataMatch(const UObject* Object, const TArray<ETextureSubType>& SubTypes)
{
    if (IsValid(Object))
    {
        const FString Actual = Object->GetPackage()->GetMetaData()->GetValue(Object, MetaDataKey);
        const FString Expected = EncodeSubTypes(SubTypes);
        return Expected.Equals(Actual);
    }
    else
    {
        return false;
    }
}

bool FTextureSubTypeUtil::EncodeInMetaData(const UObject* Object, const TArray<ETextureSubType>& SubTypes)
{
    if (IsValid(Object))
    {
        UMetaData* MetaData = Object->GetPackage()->GetMetaData();
        if (SubTypes.IsEmpty())
        {
            if (MetaData->HasValue(Object, MetaDataKey))
            {
                MetaData->RemoveValue(Object, MetaDataKey);
                ensure(Object->MarkPackageDirty());
                ensure(Object->GetOuter()->MarkPackageDirty());
            }
            return true;
        }
        else
        {
            const FString EncodedValue = EncodeSubTypes(SubTypes);
            if (EncodedValue.IsEmpty())
            {
                return false;
            }
            else
            {
                const FString Existing = MetaData->GetValue(Object, MetaDataKey);
                if (!EncodedValue.Equals(Existing))
                {
                    MetaData->SetValue(Object, MetaDataKey, *EncodedValue);
                    ensure(Object->MarkPackageDirty());
                    ensure(Object->GetOuter()->MarkPackageDirty());
                }
                return true;
            }
        }
    }
    else
    {
        return false;
    }
}

FString FTextureSubTypeUtil::EncodeSubTypes(const TArray<ETextureSubType>& SubTypes)
{
    const UEnum* Enum = StaticEnum<ETextureSubType>();
    FString Result;
    for (auto SubType : SubTypes)
    {
        if (const FName Name = Enum->GetNameByValue(static_cast<int64>(SubType)); NAME_None == Name)
        {
            Result.Reset();
            return Result;
        }
        else
        {
            Result.Append(Name.ToString());
        }
    }

    return Result;
}

FTextureSubTypeUtil::FTextureSubTypeUtil()
{
    // A private constructor so that it can not be accidentally instantiated.
}
