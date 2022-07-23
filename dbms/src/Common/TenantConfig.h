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
#include <Core/Types.h>
#include <pingcap/Config.h>
#include <pingcap/pd/IClient.h>
#include <kvproto/enginepb.pb.h>
#include <Poco/Util/LayeredConfiguration.h>

namespace DB
{

struct TenantConfig
{
    bool enabled = false;
    int id = 0;

private:
    void loadTenantIDFromClusterName(const Strings &pd_addrs, const pingcap::ClusterConfig & cluster_config, const char * cluster_name);

public:
    TenantConfig() = default;

    TenantConfig(const Poco::Util::LayeredConfiguration & config, const Strings &pd_addrs, const pingcap::ClusterConfig & cluster_config)
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
            const char * tenant_id = getenv("TIDB_TENANT_ID");
            if (tenant_id) {
                id = atoi(tenant_id);
                enabled = true;
            }
        }
        // try get tenant id from cluster name
        if (!enabled || id == 0) {
          const char *cluster_name = getenv("TIDB_CLUSTER_NAME");
          if (cluster_name) {
            loadTenantIDFromClusterName(pd_addrs, cluster_config, cluster_name);
          }
        }
        if (enabled && (id > UINT16_MAX || id == 0)) {
            throw Poco::Exception("Invalid tenant id: " + std::to_string(id));
        }
    }
};

} // namespace DB
