<?php
// $Id$

/**
 * @file
 * Provides info about the taxonomy entity.
 */

/**
 * Implement hook_entity_info().
 */
function taxonomy_entity_info() {
  $return = array(
    'taxonomy_term' => array(
      'label' => t('Taxonomy term'),
      'controller class' => 'TaxonomyTermController',
      'base table' => 'taxonomy_term_data',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'tid',
        'bundle' => 'vocabulary_machine_name',
      ),
      'bundle keys' => array(
        'bundle' => 'machine_name',
      ),
      'bundles' => array(),
    ),
  );
  foreach (taxonomy_vocabulary_get_names() as $machine_name => $vocabulary) {
    $return['taxonomy_term']['bundles'][$machine_name] = array(
      'label' => $vocabulary->name,
      'admin' => array(
        'path' => 'admin/structure/taxonomy/%taxonomy_vocabulary',
        'real path' => 'admin/structure/taxonomy/' . $vocabulary->vid,
        'bundle argument' => 3,
        'access arguments' => array('administer taxonomy'),
      ),
    );
  }
  // Add meta-data about the basic taxonomy properties.
  $properties = &$return['taxonomy_term']['properties'];
  
  $properties['tid'] = array(
    'label' => t("Term ID"),
    'description' => t("The unique ID of the taxonomy term."),
    'type' => 'integer',
  );
  $properties['vid'] = array(
    'label' => t("Vocabulary ID"),
    'description' => t("The unique ID of the vocabulary the term belongs to."),
    'setter callback' => 'drupal_property_verbatim_set',
    'type' => 'integer',
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name of the taxonomy term."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['description'] = array(
    'label' => t("Description"),
    'description' => t("The optional description of the taxonomy term."),
    'sanitize' => 'filter_xss',
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['node-count'] = array(
    'label' => t("Node count"),
    'description' => t("The number of nodes tagged with the taxonomy term."),
    'getter callback' => 'taxonomy_term_get_properties',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the taxonomy term."),
    'getter callback' => 'taxonomy_term_get_properties',
  );
  $properties['vocabulary'] = array(
    'label' => t("Vocabulary"),
    'description' => t("The vocabulary the taxonomy term belongs to."),
    'getter callback' => 'taxonomy_term_get_properties',
    'type' => 'vocabulary',
  );
  $properties['parent'] = array(
    'label' => t("Parent term"),
    'description' => t("The parent term of the taxonomy term, if one exists."),
    'getter callback' => 'taxonomy_term_get_properties',
    'type' => 'taxonomy_term',
  );

  $return['taxonomy_vocabulary'] = array(
    'label' => t('Taxonomy vocabulary'),
    'controller class' => 'TaxonomyVocabularyController',
    'base table' => 'taxonomy_vocabulary',
    'object keys' => array(
      'id' => 'vid',
    ),
    'fieldable' => FALSE,
  );

  // Add meta-data about the basic vocabulary properties.
  $properties = &$return['taxonomy_vocabulary']['properties'];
  
  // Taxonomy vocabulary related variables.
  $properties['vid'] = array(
    'label' => t("Vocabulary ID"),
    'description' => t("The unique ID of the taxonomy vocabulary."),
    'type' => 'integer',
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name of the taxonomy vocabulary."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['machine_name'] = array(
    'label' => t("Machine name"),
    'description' => t("The machine name of the taxonomy vocabulary."),
  );
  $properties['description'] = array(
    'label' => t("Description"),
    'description' => t("The optional description of the taxonomy vocabulary."),
    'setter callback' => 'drupal_property_verbatim_set',
    'sanitize' => 'filter_xss',
  );
  $properties['node-count'] = array(
    'label' => t("Node count"),
    'description' => t("The number of nodes tagged with terms belonging to the taxonomy vocabulary."),
    'getter callback' => 'taxonomy_vocabulary_get_properties',
  );
  $properties['term-count'] = array(
    'label' => t("Term count"),
    'description' => t("The number of terms belonging to the taxonomy vocabulary."),
    'getter callback' => 'taxonomy_vocabulary_get_properties',
  );
  return $return;
}

/**
 * Implement hook_entity_info_alter().
 */
function taxonomy_entity_info_alter(&$entity_info) {
  $properties = &$entity_info['node']['properties'];
  $properties['taxonomy'] = array(
    'label' => t("Taxonomy terms"),
    'description' => t("The taxonomy terms associated with this node."),
    'getter callback' => 'drupal_property_get_list',
    'type' => 'list<taxonomy_term>',
  );
}
