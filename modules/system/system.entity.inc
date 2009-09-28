<?php
// $Id: system.tokens.inc,v 1.2 2009-08-25 10:27:15 dries Exp $

/**
 * @file
 * Provides info about system-wide entities.
 */

/**
 * Implement hook_entity_info().
 */
function system_entity_info() {
  // Create a entity for dealing with drupal variables as properties.
  $types['system'] = array(
    'label' => t("System information"),
  );
  
  $properties = &$types['system']['properties'];
  
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name of the site."),
    'getter callback' => 'system_get_properties',
    'sanitize' => 'check_plain',
  );
  $properties['slogan'] = array(
    'label' => t("Slogan"),
    'description' => t("The slogan of the site."),
    'getter callback' => 'system_get_properties',
    'sanitize' => 'check_plain',
  );
  $properties['mission'] = array(
    'label' => t("Mission"),
    'description' => t("The optional 'mission' of the site."),
    'getter callback' => 'system_get_properties',
    'sanitize' => 'filter_xss',
  );
  $properties['mail'] = array(
    'label' => t("Email"),
    'description' => t("The administrative email address for the site."),
    'getter callback' => 'system_get_properties',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the site's front page."),
    'getter callback' => 'system_get_properties',
  );
  $properties['login-url'] = array(
    'label' => t("Login page"),
    'description' => t("The URL of the site's login page."),
    'getter callback' => 'system_get_properties',
  );

  // Describe files.
  $types['file'] = array(
    'label' => t('File'),
    'base table' => 'file',
    'object keys' => array(
      'id' => 'fid',
    ),
    'static cache' => FALSE,
  );
  
  $properties = &$types['file']['properties'];

  $properties['fid'] = array(
    'label' => t("File ID"),
    'description' => t("The unique ID of the uploaded file."),
  );
  $properties['uid'] = array(
    'label' => t("User ID"),
    'description' => t("The unique ID of the user who owns the file."),
  );
  $properties['name'] = array(
    'label' => t("File name"),
    'description' => t("The name of the file on disk."),
    'getter callback' => 'system_get_file_properties',
  );
  $properties['path'] = array(
    'label' => t("Path"),
    'description' => t("The location of the file on disk."),
    'getter callback' => 'system_get_file_properties',
  );
  $properties['mime'] = array(
    'label' => t("MIME type"),
    'description' => t("The MIME type of the file."),
    'getter callback' => 'system_get_file_properties',
  );
  $properties['size'] = array(
    'label' => t("File size"),
    'description' => t("The size of the file, in kilobytes."),
    'getter callback' => 'system_get_file_properties',
    'type' => 'integer',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The web-accessible URL for the file."),
    'getter callback' => 'system_get_file_properties',
  );
  $properties['timestamp'] = array(
    'label' => t("Timestamp"),
    'description' => t("The date the file was most recently changed."),
    'type' => 'date',
  );
  $properties['owner'] = array(
    'label' => t("Owner"),
    'description' => t("The user who originally uploaded the file."),
    'type' => 'user',
    'getter callback' => 'system_get_file_properties',
  );
  
  return $types;
}
