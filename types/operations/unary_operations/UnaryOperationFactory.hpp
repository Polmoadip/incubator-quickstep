/**
 *   Copyright 2011-2015 Quickstep Technologies LLC.
 *   Copyright 2015 Pivotal Software, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 **/

#ifndef QUICKSTEP_TYPES_OPERATIONS_UNARY_OPERATIONS_UNARY_OPERATION_FACTORY_HPP_
#define QUICKSTEP_TYPES_OPERATIONS_UNARY_OPERATIONS_UNARY_OPERATION_FACTORY_HPP_

#include "types/operations/unary_operations/UnaryOperationID.hpp"
#include "utility/Macros.hpp"

namespace quickstep {

class UnaryOperation;
namespace serialization { class UnaryOperation; }

/** \addtogroup Types
 *  @{
 */

/**
 * @brief All-static factory object that provides access to UnaryOperations.
 **/
class UnaryOperationFactory {
 public:
  /**
   * @brief Convenience factory method to get a pointer to a UnaryOperation
   *        from that UnaryOperation's ID.
   *
   * @param id The ID of the desired UnaryOperation.
   * @return The UnaryOperation corresponding to id.
   **/
  static const UnaryOperation& GetUnaryOperation(const UnaryOperationID id);

  /**
   * @brief Get a reference to a UnaryOperation from that UnaryOperation's
   *        serialized Protocol Buffer representation.
   *
   * @param proto A serialized Protocol Buffer representation of a UnaryOperation,
   *        originally generated by getProto().
   * @return The UnaryOperation described by proto.
   **/
  static const UnaryOperation& ReconstructFromProto(const serialization::UnaryOperation &proto);

  /**
   * @brief Check whether a serialization::UnaryOperation is fully-formed and
   *        all parts are valid.
   *
   * @param proto A serialized Protocol Buffer representation of a UnaryOperation,
   *        originally generated by getProto().
   * @return Whether proto is fully-formed and valid.
   **/
  static bool ProtoIsValid(const serialization::UnaryOperation &proto);

 private:
  UnaryOperationFactory();

  DISALLOW_COPY_AND_ASSIGN(UnaryOperationFactory);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_TYPES_OPERATIONS_UNARY_OPERATIONS_UNARY_OPERATION_FACTORY_HPP_
