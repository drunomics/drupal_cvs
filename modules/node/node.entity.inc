<?php
// $Id$

/**
 * @file
 * Provides info about the node entity.
 */


/**
 * Implement hook_entity_info().
 */
function node_entity_info() {
  $return = array(
    'node' => array(
      'label' => t('Node'),
      'controller class' => 'NodeController',
      'base table' => 'node',
      'revision table' => 'node_revision',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'nid',
        'revision' => 'vid',
        'bundle' => 'type',
      ),
      // Node.module handles its own caching.
      // 'cacheable' => FALSE,
      'bundles' => array(),
      'name property' => 'title',
    ),
  );
  // Bundles must provide a human readable name so we can create help and error
  // messages, and the path to attach Field admin pages to.
  foreach (node_type_get_names() as $type => $name) {
    $return['node']['bundles'][$type] = array(
      'label' => $name,
      'admin' => array(
        'path' => 'admin/structure/types/manage/' . str_replace('_', '-', $type),
        'access arguments' => array('administer content types'),
      ),
    );
  }
  // Add meta-data about the basic node properties.
  $properties = &$return['node']['properties'];
  
  $properties['nid'] = array(
    'label' => t("Node ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node."),
  );
  $properties['vid'] = array(
    'label' => t("Revision ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node's latest revision."),
  );
  $properties['tnid'] = array(
    'label' => t("Translation set ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the original-language version of this node, if one exists."),
  );
  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the author of the node."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['type'] = array(
    'label' => t("Content type"),
    'description' => t("The type of the node."),
  );
  $properties['type-name'] = array(
    'label' => t("Content type name"),
    'description' => t("The human-readable name of the node type."),
    'getter callback' => 'node_get_properties',
  );
  $properties['title'] = array(
    'label' => t("Title"),
    'description' => t("The title of the node."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['language'] = array(
    'label' => t("Language"),
    'description' => t("The language the node is written in."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the node."),
    'getter callback' => 'node_get_properties',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The URL of the node's edit page."),
    'getter callback' => 'node_get_properties',
  );
  $properties['created'] = array(
    'label' => t("Date created"),
    'type' => 'date',
    'description' => t("The date the node was posted."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['changed'] = array(
    'label' => t("Date changed"),
    'type' => 'date',
    'description' => t("The date the node was most recently updated."),
  );
  $properties['author-name'] = array(
    'label' => t("Author name"),
    'description' => t("The node author's name."),
    'getter callback' => 'node_get_properties',
  );
  $properties['author'] = array(
    'label' => t("Author"),
    'type' => 'user',
    'description' => t("The author of the node."),
    'getter callback' => 'node_get_properties',
  );

  return $return;
}

