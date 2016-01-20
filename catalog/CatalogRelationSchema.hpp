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

#ifndef QUICKSTEP_CATALOG_CATALOG_RELATION_SCHEMA_HPP_
#define QUICKSTEP_CATALOG_CATALOG_RELATION_SCHEMA_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "catalog/CatalogAttribute.hpp"
#include "catalog/CatalogTypedefs.hpp"
#include "utility/Macros.hpp"
#include "utility/PtrVector.hpp"
#include "utility/StringUtil.hpp"

#include "glog/logging.h"

namespace quickstep {

class CatalogDatabase;

/** \addtogroup Catalog
 *  @{
 */

/**
 * @brief A relation in a database. This base class only includes information
 *        about the relation's schema. See also CatalogRelation, which augments
 *        this with additional information about physical blocks.
 **/
class CatalogRelationSchema {
 public:
  typedef std::unordered_map<std::string, CatalogAttribute*>::size_type size_type;
  typedef PtrVector<CatalogAttribute, true>::const_skip_iterator const_iterator;

  /**
   * @brief Destructor which recursively destroys children.
   **/
  virtual ~CatalogRelationSchema() = 0;

  /**
   * @brief Get the parent database.
   *
   * @return Parent database.
   **/
  const CatalogDatabase& getParent() const {
    return *parent_;
  }

  /**
   * @brief Get a mutable pointer to the parent database.
   *
   * @return Parent database.
   **/
  CatalogDatabase* getParentMutable() {
    return parent_;
  }

  /**
   * @brief Get this relation's ID.
   *
   * @return This relation's ID.
   **/
  relation_id getID() const {
    return id_;
  }

  /**
   * @brief Get this relation's name.
   *
   * @return This relation's name.
   **/
  const std::string& getName() const {
    return name_;
  }

  /**
   * @brief Check whether this relation is temporary or permanent.
   *
   * @return True if this relation is temporary, false otherwise.
   **/
  bool isTemporary() const {
    return temporary_;
  }

  /**
   * @brief Check whether an attribute with the given name exists.
   *
   * @param attr_name The name to check for.
   * @return Whether the attribute exists.
   **/
  bool hasAttributeWithName(const std::string &attr_name) const {
    return (attr_map_.find(ToLower(attr_name)) != attr_map_.end());
  }

  /**
   * @brief Check whether an attribute with the given id exists.
   *
   * @param id The id to check for.
   * @return Whether the attribute exists.
   **/
  bool hasAttributeWithId(const attribute_id id) const {
    return (idInRange(id) && !attr_vec_.elementIsNull(id));
  }

  /**
   * @brief Get an attribute by name. The search is case-insensitive.
   *
   * @param attr_name The name to search for.
   * @return The attribute with the given name. NULL if the attribute is not found.
   **/
  const CatalogAttribute* getAttributeByName(const std::string &attr_name) const;

  /**
   * @brief Get a mutable pointer to an attribute by name. The search is case-insensitive.
   *
   * @param attr_name The name to search for.
   * @return The attribute with the given name. NULL if the attribute is not found.
   **/
  CatalogAttribute* getAttributeByNameMutable(const std::string &attr_name);

  /**
   * @brief Get an attribute by ID.
   *
   * @param id The id to search for.
   * @return The attribute with the given ID.
   **/
  const CatalogAttribute* getAttributeById(const attribute_id id) const {
    if (hasAttributeWithId(id)) {
      return &attr_vec_[id];
    } else {
      return nullptr;
    }
  }

  /**
   * @brief Get a mutable pointer to an attribute by ID.
   *
   * @param id The id to search for.
   * @return The attribute with the given ID.
   * @exception AttributeIdNotFound No attribute with the given ID exists.
   **/
  CatalogAttribute* getAttributeByIdMutable(const attribute_id id) {
    if (hasAttributeWithId(id)) {
      return &(attr_vec_[id]);
    } else {
      return nullptr;
    }
  }

  /**
   * @brief Add a new attribute to the relation. If the attribute already has
   *        an ID and/or parent, it will be overwritten.
   *
   * @param new_attr The attribute to be added.
   * @return The id assigned to the attribute.
   * @exception AttributeNameCollision An attribute with the same name as
   *            new_attr is already present in the relation.
   **/
  attribute_id addAttribute(CatalogAttribute *new_attr);

  /**
   * @brief Check whether tuples of the relation are variable-length.
   *
   * @return Whether the relation is variable length (i.e. whether any child
   *         attributes are variable length).
   **/
  bool isVariableLength() const {
    return num_variable_length_attributes_ > 0;
  }

  /**
   * @brief Check how many attributes of the relation are variable-length.
   *
   * @return The number of variable-length attributes in the relation.
   **/
  inline unsigned numVariableLengthAttributes() const {
    return num_variable_length_attributes_;
  }

  /**
   * @brief Get the index of a variable-length attribute among all the
   *        variable-length attributes in this relation.
   *
   * @param id The id of the desired attribute.
   * @return The attribute's index amongst all of the variable-length
   *         attributes, or -1 if the attribute is fixed-length.
   **/
  inline int getVariableLengthAttributeIndex(const attribute_id id) const {
    DCHECK(hasAttributeWithId(id));
    return variable_length_attribute_indices_[id];
  }

  /**
   * @brief Get the maximum length of tuples of this relation, in bytes.
   *
   * @return The maximum length of tuples of this relation, in bytes (equal to
   *         getFixedByteLength() if relation is fixed-length).
   **/
  std::size_t getMaximumByteLength() const {
    return max_byte_length_;
  }

  /**
   * @brief Get the minimum length of tuples of this relation, in bytes.
   *
   * @return The minimum length of tuples of this relation, in bytes (equal
   *         to getFixedByteLength() and getMaximumByteLength() if relation is
   *         fixed-length).
   **/
  std::size_t getMinimumByteLength() const {
    return min_byte_length_;
  }

  /**
   * @brief Get the estimated average length of tuples of this relation, in
   *        bytes.
   *
   * @return The estimated average length of tuples of this relation, in bytes
   *         (equal to getFixedByteLength(), getMinimumByteLength(), and
   *         getMaximumByteLength() if relation is fixed-length).
   **/
  std::size_t getEstimatedByteLength() const {
    return estimated_byte_length_;
  }

  /**
   * @brief Get the total length of the fixed-length attributes in this
   *        relation, in bytes.
   *
   * @return The total length of fixed-length attributes in this relation, in
   *         bytes.
   **/
  std::size_t getFixedByteLength() const {
    return fixed_byte_length_;
  }

  /**
   * @brief Get the total maximum length of the variable-length attributes of
   *        this relation, in bytes.
   *
   * @return The total maximum length of the variable-length attributes of this
   *         relation, in bytes (0 if the relation is fixed-length).
   **/
  std::size_t getMaximumVariableByteLength() const {
    return max_variable_byte_length_;
  }

  /**
   * @brief Get the total minimum length of the variable-length attributes of
   *        this relation, in bytes.
   *
   * @return The total minimum length of the variable-length attributes of this
   *         relation, in bytes (0 if the relation is fixed-length).
   **/
  std::size_t getMinimumVariableByteLength() const {
    return min_variable_byte_length_;
  }

  /**
   * @brief Get the total minimum length of the non-nullable variable-length
   *        attributes of this relation, in bytes.
   *
   * @return The total minimum length of the non-nullable variable-length
   *         attributes of this relation, in bytes (0 if the relation is
   *         fixed-length).
   **/
  std::size_t getMinimumVariableByteLengthExcludingNullable() const {
    return min_variable_byte_length_excluding_nullable_;
  }

  /**
   * @brief Get the estimated average length of all the variable-length
   *        attributes of this relation, in bytes.
   *
   * @return The total estimated average length of variable-length attributes
   *         of this relation, in bytes (0 if the relation is fixed-length).
   **/
  std::size_t getEstimatedVariableByteLength() const {
    return estimated_variable_byte_length_;
  }

  /**
   * @brief Get the byte offset of a fixed-length attribute in this relation.
   * @warning This method should only be called for attributes which are
   *          fixed-length. For debug builds, this is checked with an assert.
   *          For release builds, it is unchecked.
   *
   * @param id The id of the desired attribute.
   * @return The byte-offset of the specified fixed-length attribute (as it
   *         would be in a conventional row-store) in this relation.
   **/
  inline std::size_t getFixedLengthAttributeOffset(const attribute_id id) const {
    DCHECK(hasAttributeWithId(id));
    DCHECK_EQ(-1, variable_length_attribute_indices_[id]);
    return fixed_length_attribute_offsets_[id];
  }

  /**
   * @brief Check whether any attributes of the relation are nullable.
   *
   * @return Whether the relation has any nullable attributes.
   **/
  inline bool hasNullableAttributes() const {
    return num_nullable_attributes_ > 0;
  }

  /**
   * @brief Check how many attributes of the relation are nullable.
   *
   * @return The number of nullable attributes in the relation.
   **/
  inline unsigned numNullableAttributes() const {
    return num_nullable_attributes_;
  }

  /**
   * @brief Get the index of a nullable attribute among all the nullable
   *        attributes in this relation.
   *
   * @param id The id of the desired attribute.
   * @return The attribute's index amongst all of the nullable attributes
   *         (intended for indexing into a NULL bitmap), or -1 if the attribute
   *         is not nullable.
   **/
  inline int getNullableAttributeIndex(const attribute_id id) const {
    DCHECK(hasAttributeWithId(id));
    return nullable_attribute_indices_[id];
  }

  /**
   * @brief Get the number of child attributes.
   *
   * @return The number of child attributes.
   **/
  size_type size() const {
    return attr_map_.size();
  }

  /**
   * @brief Determine whether the sequence of attribute IDs has gaps in it.
   *
   * @return Whether the sequence of attribute IDs has any gaps.
   **/
  bool gapsInAttributeSequence() const {
    return (attr_map_.size() != attr_vec_.size());
  }

  /**
   * @brief Get the highest attribute ID in this relation.
   *
   * @return The highest attribute ID in this relation (-1 if no attributes
   *         exist).
   **/
  attribute_id getMaxAttributeId() const;

  /**
   * @brief Get an iterator at the beginning of the child attributes.
   *
   * @return An iterator on the first child attribute.
   **/
  inline const_iterator begin() const {
    return attr_vec_.begin_skip();
  }

  /**
   * @brief Get an iterator at one-past-the-end of the child attributes.
   *
   * @return An iterator one-past-the-end of the child attributes.
   **/
  inline const_iterator end() const {
    return attr_vec_.end_skip();
  }

 protected:
  /**
   * @brief Create a new relation.
   *
   * @param parent The database this relation belongs to.
   * @param name This relation's name.
   * @param id This relation's ID (defaults to -1, which means invalid/unset).
   * @param temporary Whether this relation is temporary (stores an
   *        intermediate result during query processing).
   **/
  CatalogRelationSchema(CatalogDatabase* parent,
                        const std::string &name,
                        const relation_id id = -1,
                        const bool temporary = false)
      : parent_(parent),
        id_(id),
        name_(name),
        temporary_(temporary),
        num_nullable_attributes_(0),
        num_variable_length_attributes_(0),
        max_byte_length_(0),
        min_byte_length_(0),
        estimated_byte_length_(0),
        fixed_byte_length_(0),
        max_variable_byte_length_(0),
        min_variable_byte_length_(0),
        min_variable_byte_length_excluding_nullable_(0),
        estimated_variable_byte_length_(0),
        current_nullable_attribute_index_(-1),
        current_variable_length_attribute_index_(-1) {
  }

  /**
   * @brief Set the parent CatalogDatabase of this relation. Used by
   *        CatalogDatabase (a friend of this class) when adding a new
   *        relation.
   *
   * @param parent The new parent for this CatalogRelationSchema.
   **/
  void setParent(CatalogDatabase *parent) {
    parent_ = parent;
  }

  /**
   * @brief Set the ID of this relation. Used by CatalogDatabase (a friend of
   *        this class) when adding a new relation.
   *
   * @param id The new ID for this CatalogRelationSchema.
   **/
  void setID(const relation_id id) {
    id_ = id;
  }

  /**
   * @brief Check whether an attribute_id is within the range of IDs contained
   *        in this CatalogRelationSchema.
   *
   * @param id The id to check.
   * @return true if id is in range, false otherwise.
   **/
  bool idInRange(const attribute_id id) const {
    return ((id >= 0)
            && (static_cast<PtrVector<CatalogAttribute>::size_type>(id) < attr_vec_.size()));
  }

  CatalogDatabase *parent_;

  // The relation id in CatalogDatabase.
  relation_id id_;

  // The relation name.
  const std::string name_;

  // Whether a temporary relation.
  const bool temporary_;

  // A vector of attributes, and NULL if it has removed from the relation
  // schema.
  PtrVector<CatalogAttribute, true> attr_vec_;

  // A map from a lower case attribute name to a pointer to the attribute.
  std::unordered_map<std::string, CatalogAttribute*> attr_map_;

  // These schema parameters are cached so we don't have to recalculate them
  // every time.
  unsigned num_nullable_attributes_;
  unsigned num_variable_length_attributes_;
  std::size_t max_byte_length_,
              min_byte_length_,
              estimated_byte_length_,
              fixed_byte_length_,
              max_variable_byte_length_,
              min_variable_byte_length_,
              min_variable_byte_length_excluding_nullable_,
              estimated_variable_byte_length_;
  std::vector<std::size_t> fixed_length_attribute_offsets_;

  // Entries are -1 for non-nullable attributes.
  std::vector<int> nullable_attribute_indices_;
  int current_nullable_attribute_index_;

  std::vector<int> variable_length_attribute_indices_;
  int current_variable_length_attribute_index_;

 private:
  friend class CatalogDatabase;

  DISALLOW_COPY_AND_ASSIGN(CatalogRelationSchema);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_CATALOG_CATALOG_RELATION_SCHEMA_HPP_
