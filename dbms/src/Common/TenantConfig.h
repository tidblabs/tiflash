// Copyright 2022 PingCAP, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once
#include <Poco/Util/LayeredConfiguration.h>

namespace DB
{

struct TenantConfig
{
    bool enabled = false;
    int id = 0;

public:
    TenantConfig() = default;

    //TenantConfig(const TenantConfig &from) : enabled(from.enabled), id(from.id) {}

    TenantConfig(Poco::Util::LayeredConfiguration & config)
    {
        if (config.has("tenant"))
        {
            if (config.has("tenant.is-tenant"))
            {
                enabled = config.getBool("tenant.is-tenant");
            }

            if (config.has("tenant.tenant-id"))
            {
                id = config.getInt("tenant.tenant-id");
            }
        }

        // try loading config from env
        if (!enabled || id == 0) {
            const char * tenant_id = getenv("TIFLASH_TENANT_ID");
            if (tenant_id) {
                id = atoi(tenant_id);
                enabled = true;
            }
        }
        if (enabled && (id > UINT16_MAX || id == 0)) {
            throw Exception("Invalid tenant id: " + std::to_string(id));
        }
    }
};

} // namespace DB
