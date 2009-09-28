<?php
// $Id$

/**
 * @file
 * Provides the drupal property wrapper classes.
 */

/**
 * A common interface for all property wrappers. Each wrapper can be used
 * to derive or set values depending on the concrete wrappers, so there are
 * wrappers to get/set entity properties, to format the property values and to
 * deal with multiple value properties. If possible, a wrapper always returns
 * another wrapper to allow chained usage like:
 * @code
 *   echo $wrappedNode->author->login->since;
 * @endcode
 */
interface DrupalPropertyWrapperInterface extends IteratorAggregate {

  /**
   * Constructor.
   *
   * @see drupal_get_property_wrapper()
   */
  public function __construct($type, &$data, array $options = array());

  /**
   * Get the data wrapped by this object.
   */
  public function get();

  /**
   * Magic method: Get a derived value.
   */
  public function __get($name);

  /**
   * Magic method: Check whether a derived value of this name is supported.
   */
  public function __isset($name);

}

/**
 * Provides a wrapper for entities which eases dealing with entity properties.
 *
 * The wrapper eases applying getter and setter callbacks of entity properties
 * specified in hook_entity_info() while respecting the specified options. Any
 * possible bundles or entity tags of the wrapped entity are detected, so any
 * associated properties can be used.
 */
class DrupalEntityPropertyWrapper implements DrupalPropertyWrapperInterface {

  protected $entityType;
  protected $entity;
  protected $info;
  protected $options;
  protected $cache = array();

  /**
   * Construct a new DrupalEntityPropertyWrapper object.
   *
   * @param $entityType
   *   The type of the passed entity.
   * @param $entity
   *   The entity with which properties we deal.
   * @param $options
   *   (optional) A keyed array of options, passed through to any returned
   *   wrappers. Used by this wrapper are:
   *   - language: A language object to be used when getting locale-sensitive
   *     properties.
   *   - sanitize: A boolean flag indicating that textual properties should be
   *     sanitized for display to a web browser. Defaults to FALSE.
   *   - absolute: Whether generated URLs should be absolute. Defaults to TRUE.
   */
  public function __construct($entityType, &$entity, array $options = array()) {
    $this->entityType = $entityType;
    $this->entity = $entity;
    $this->info = entity_get_info($entityType) + array('properties' => array(), 'default tags' => array(), 'name property' => 'name');
    $this->options = $options + array('sanitize' => FALSE, 'language' => NULL, 'absolute' => TRUE);
  
    // Add in properties from the bundle or tags.
    if (!empty($this->info['fieldable']) && is_object($entity)) {
      list($id, $vid, $bundle) = field_extract_ids($entityType, $entity);
      $bundle_info = (array)$this->info['bundles'][$bundle] + array('properties' => array(), 'default tags' => array());
      $this->info['properties'] += $bundle_info['properties'];
      $this->info['default tags'] = array_merge($this->info['default tags'], $bundle_info['default tags']);
    }
    foreach ($this->getTags() as $tag) {
      if (isset($this->info['tags'][$tag]['properties'])) {
        $this->info['properties'] += $this->info['tags'][$tag]['properties'];
      }
    }
  }

  /**
   * Returns all tags assigned to the wrapped entity.
   *
   * @return
   *   An array of tags.
   */
  public function getTags() {
    if (is_object($this->entity) && isset($this->entity->entityTags)) {
      return $this->entity->entityTags;
    }
    elseif (is_array($this->entity) && isset($this->entity['entity tags'])) {
      return $this->entity['entity tags'];
    }
    // In case the entity doesn't have tags set, fallback to the defaults.
    return $this->info['default tags'];
  }

  /**
   * Gets the info about the given property.
   *
   * @param $name
   *   The name of the property.
   * @throws DrupalPropertyWrapperException
   *   If there is no such property.
   * @return
   *   An array of info about the property.
   */
  public function getPropertyInfo($name) {
    if (!isset($this->info['properties'][$name])) {
      throw new DrupalPropertyWrapperException('Unknown entity property ' . check_plain($name) . '.');
    }
    $info = $this->info['properties'][$name] + array(
      'type' => 'text',
      'bundle' => NULL,
      'tags' => array(),
      'formats' => array(),
    );
    $info += array('sanitize' => (empty($info['getter callback']) && $info['type'] == 'text' ? 'check_plain' : NULL));
    $info += (array)drupal_get_datatype_info($info['type']);
    if ($contained_type = drupal_list_extract_type($info['type'])) {
      $info['item']['type'] = $contained_type;
      $info['item'] += (array)drupal_get_datatype_info($contained_type);
    }
    return $info;
  }

  /**
   * Gets the info about the wrapped entity.
   */
  public function getInfo() {
    return $this->info;
  }

  /**
   * Magic method: Get a property.
   *
   * @return
   *   An instance of DrupalPropertyWrapperInterface.
   */
  public function __get($name) {
    // Look it up in the cache if possible.
    if (!array_key_exists($name, $this->cache)) {
      $info = $this->getPropertyInfo($name);

      if (!empty($info['getter callback']) && function_exists($info['getter callback'])) {
        $this->cache[$name] = $info['getter callback']($this->entity, $this->options, $name, $this->entityType);
      }
      elseif (is_array($this->entity) && isset($this->entity[$name])) {
        $this->cache[$name] = $this->entity[$name];
      }
      elseif (is_object($this->entity) && isset($this->entity->$name)) {
        $this->cache[$name] = $this->entity->$name;
      }
      else {
        throw new DrupalPropertyWrapperException('Entity property ' . check_plain($name) . " isn't set.");
      }
      // Sanitize values.
      if (isset($this->cache[$name]) && !empty($this->options['sanitize']) && !empty($info['sanitize']) && function_exists($info['sanitize'])) {
        $this->cache[$name] = $info['sanitize']($this->cache[$name]);
      }
      // Return another wrapper to support chained usage.
      $options = array_intersect_key($info, drupal_map_assoc(array('default format', 'formats', 'item'))) + $this->options;
      $context = array('entityWrapper' => $this, 'entity' => $this->entity, 'entity type' => $this->entityType, 'property' => $name);
      $this->cache[$name] = drupal_get_property_wrapper($info['type'], $this->cache[$name], $options, $context);
    }
    return $this->cache[$name];
  }

  /**
   * Magic method: Set a property.
   */
  public function __set($name, $value) {
    $info = $this->getPropertyInfo($name);
    if (!empty($info['setter callback']) && function_exists($info['setter callback'])) {
      unset($this->cache[$name]);
      return $info['setter callback']($this->entity, $name, $value, $this->entityType);
    }
    throw new DrupalPropertyWrapperException('Entity property ' . check_plain($name) . " doesn't support writing.");
  }

  /**
   * Magic method: isset() can be used to check if a property is known.
   */
  public function __isset($name) {
    return isset($this->info['properties'][$name]);
  }

  /**
   * Get the entity wrapped by this object.
   */
  public function get() {
    return $this->entity;
  }

  public function getIterator() {
    return new ArrayIterator(array_keys($this->info['properties']));
  }
 
  public function __toString() {
    if (isset($this->{$this->info['name property']})) {
      // Return the default format of the name property, if specified.
      return (string)$this->{$this->info['name property']};
    }
    return $this->entityType;
  }
}

/**
 * Wraps an actual property to ease formating properties and dealing with lists
 * of values.
 *
 * This wrapper can be used to easily apply formats, which have to be specified
 * in hook_datatype_info() or specificly for the wrapped property in
 * hook_entity_info(). If the wrapped data is a list of data, its numerical
 * indexes may be used to retrieve wrappers for the list items. For that this
 * wrapper may be also used like a usual numerically indexed array.
 *
 * If the wrapper gets converted to string, the default format is applied.
 */
class DrupalPropertyValueWrapper implements DrupalPropertyWrapperInterface, ArrayAccess {

  protected $type;
  protected $isList = FALSE;
  protected $data;
  protected $options;
  protected $context;
  protected $cache = array();

  /**
   * Construct a new DrupalPropertyValueWrapper object.
   *
   * @param $type
   *   The type of the passed data.
   * @param $data
   *   The data to format. If the data type is a list, the data has to be a
   *   numerically indexed array.
   * @param $options
   *   (optional) A keyed array of options, passed through to any returned
   *   wrappers. Used by this wrapper are:
   *   - language: A language object to be used when generating
   *     locale-sensitive formats.
   *   - formats: An array of further formats for this property
   *     as defined in hook_entity_info().
   *   - default format: Customize the default format.
   *   - absolute: Whether generated URLs should be absolute. Defaults to TRUE.
   *   - item: An array of options to be used for contained list items.
   * @param $context
   *   (optional) An array of contextual information which format callbacks
   *   can make use of. Contains 'entityWrapper', 'entity', 'entity_type' and
   *   'property' when constructed by the DrupalEntityPropertyWrapper.
   */
  public function __construct($type, &$data, array $options = array(), $context = array()) {
    $this->type = $type;
    $this->data = $data;
    $this->options = $options + array('formats' => array(), 'item' => array(), 'absolute' => TRUE, 'language' => NULL);
    $this->applyDataTypeDefaults($this->options, $type);
    // Add in list specific formats, if we are dealing with a list.
    if ($this->isList = drupal_list_extract_type($type)) {
      $this->applyDataTypeDefaults($this->options, 'list');
    }
    $this->context = array('wrapper' => $this) + $context;
  }
  
  protected function applyDataTypeDefaults(&$info, $type) {
    $type_info = drupal_get_datatype_info($type) + array('formats' => array());
    $info += $type_info;
    $info['formats'] += $type_info['formats'];
  }

  /**
   * Magic method: Get a list item (numeric name) or apply a format (else).
   */
  public function __get($name) {
    // Look it up in the cache if possible.
    if (!array_key_exists($name, $this->cache)) {
      if (is_numeric($name) && $this->isList) {
        if (!isset($this->data[$name])) {
          throw new DrupalPropertyWrapperException('Array entry ' . check_plain($name) . " isn't set.");
        }
        // Return another wrapper for the list item.
        $options = $this->options['item'] + array_intersect_key($this->options, drupal_map_assoc(array('absolute', 'language', 'sanitize')));
        $this->context['delta'] = $name;
        $this->cache[$name] = drupal_get_property_wrapper($this->isList, $this->data[$name], $options, $this->context);
      }
      else {
        if (!isset($this->options['formats'][$name])) {
          throw new DrupalPropertyWrapperException('Unknown format ' . check_plain($name) . '.');
        }
        $this->cache[$name] = token_format($this->data, $this->type, $name, $this->options, $this->context);
      }
    }
    return $this->cache[$name];
  }

  /**
   * Magic method: Check if a format is known or a list item exists.
   */
  public function __isset($name) {
    return (is_numeric($name) && $this->isList && isset($this->data[$name])) || isset($this->options['formats'][$name]);
  }

  /**
   * Get the data wrapped by this object.
   */
  public function get() {
    return $this->data;
  }

  /**
   * If we wrap a list, we return an iterator over the data list.
   */
  public function getIterator() {
    return $this->isList ? new ArrayIterator(array_keys($this->data)) : array();
  }

  /**
   * For converting to a string use the default format, if any.
   */
  public function __toString() {
    return (string)token_format($this->data, $this->type, NULL, $this->options, $this->context);
  }

  /**
   * Implement the ArrayAccess interface.
   */
  public function offsetGet($offset) {
    return $this->__get((int)$offset);
  }

  public function offsetExists($offset) {
    return $this->isList && isset($this->data[$offset]);
  }

  public function offsetSet($offset, $value) {
    if ($this->isList && is_numeric($offset)) {
      $this->data[$offset] = $value;
      $this->updateParentWrapper();
    }
  }

  public function offsetUnset($offset) {
    if ($this->isList && is_numeric($offset)) {
      unset($this->data[$offset]);
      $this->updateParentWrapper();
    }
  }

  /**
   * Also update the value in the parent wrapped entity, if there is one.
   */
  protected function updateParentWrapper() {
    if (!empty($this->context['entityWrapper'])) {
      $this->context['entityWrapper']->{$this->context['property']} = $this->data;
    }
  }
}

/**
 * Provide a separate Exception so it can be caught separately.
 */
class DrupalPropertyWrapperException extends Exception {

}

/**
 * Sets the property to the given value. May be used as 'setter callback'.
 */
function drupal_property_verbatim_set(&$entity, $name, $value) {
  if (is_array($entity)) {
    $entity[$name] = $value;
  }
  elseif (is_object($entity)) {
    $entity->$name = $value;
  }
}

/**
 * Getter callback for getting an array. Makes sure it's numerically indexed.
 */
function drupal_property_get_list($entity, $options, $name) {
  return isset($entity->$name) ? array_values($entity->$name) : array();
}
