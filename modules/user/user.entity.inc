<?php
// $Id$

/**
 * @file
 * Provides info about the user entity.
 */

/**
 * Implement hook_entity_info().
 */
function user_entity_info() {
  $return = array(
    'user' => array(
      'label' => t('User'),
      'controller class' => 'UserController',
      'base table' => 'users',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'uid',
      ),
      'path' => 'user',
      'rdf-type' => array('sioc:User'),
      'bundles' => array(
        'user' => array(
          'label' => t('User'),
          'admin' => array(
            'path' => 'admin/config/people/accounts',
            'access arguments' => array('administer users'),
          ),
        ),
      ),
    ),
  );
  // Add meta-data about the user properties.
  $properties = &$return['user']['properties'];

  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the user account."),
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The login name of the user account."),
    'getter callback' => 'user_get_properties',
    'setter callback' => 'drupal_property_verbatim_set',
    'rdf-property' => array('foaf:name'),
  );
  $properties['mail'] = array(
    'label' => t("Email"),
    'description' => t("The email address of the user account."),
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the account profile page."),
    'getter callback' => 'user_get_properties',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The url of the account edit page."),
    'getter callback' => 'user_get_properties',
  );
  $properties['login'] = array(
    'label' => t("Last login"),
    'description' => t("The date the user last logged in to the site."),
    'type' => 'date',
  );
  $properties['created'] = array(
    'label' => t("Created"),
    'description' => t("The date the user account was created."),
    'type' => 'date',
  );
  return $return;
}

