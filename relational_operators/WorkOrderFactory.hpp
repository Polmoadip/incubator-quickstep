/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 **/

#ifndef QUICKSTEP_RELATIONAL_OPERATORS_WORK_ORDER_FACTORY_HPP_
#define QUICKSTEP_RELATIONAL_OPERATORS_WORK_ORDER_FACTORY_HPP_

#include "utility/Macros.hpp"

#include "tmb/id_typedefs.h"

namespace tmb { class MessageBus; }

namespace quickstep {

class CatalogDatabaseLite;
class QueryContext;
class StorageManager;
class WorkOrder;

namespace serialization { class WorkOrder; }

/** \addtogroup RelationalOperators
 *  @{
 */

/**
 * @brief All-static factory object that provides access to WorkOrder.
 **/
class WorkOrderFactory {
 public:
  /**
   * @brief Get a pointer to a WorkOrder from its serialized Protocol Buffer
   *        form.
   *
   * @param proto The Protocol Buffer representation of a WorkOrder object,
   *        originally generated by RelationalOperator::getAllWorkOrders.
   * @param catalog_database The database to resolve relation and attribute
   *        references in.
   * @param query_context A pointer to QueryContext.
   * @param storage_manager The StorageManager to use.
   * @param shiftboss_client_id The TMB client id of Shiftboss.
   * @param bus A pointer to the TMB.
   *
   * @return A new WorkOrder reconstructed from the supplied Protocol Buffer.
   **/
  static WorkOrder* ReconstructFromProto(const serialization::WorkOrder &proto,
                                         CatalogDatabaseLite *catalog_database,
                                         QueryContext *query_context,
                                         StorageManager *storage_manager,
                                         const tmb::client_id shiftboss_client_id,
                                         tmb::MessageBus *bus);

  /**
   * @brief Check whether a serialization::WorkOrder is fully-formed and
   *        all parts are valid.
   *
   * @param proto A serialized Protocol Buffer representation of a WorkOrder.
   * @param catalog_database The database to resolve relation and attribute
   *        references in.
   * @param query_context The QueryContext to use.
   *
   * @return Whether proto is fully-formed and valid.
   **/
  static bool ProtoIsValid(const serialization::WorkOrder &proto,
                           const CatalogDatabaseLite &catalog_database,
                           const QueryContext &query_context);

 private:
  // Undefined default constructor. Class is all-static and should not be
  // instantiated.
  WorkOrderFactory();

  DISALLOW_COPY_AND_ASSIGN(WorkOrderFactory);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_RELATIONAL_OPERATORS_WORK_ORDER_FACTORY_HPP_
