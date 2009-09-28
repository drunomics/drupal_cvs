<?php
// $Id$

/**
 * @file
 * Provides info about the comment entity.
 */


/**
 * Implement hook_entity_info() {
 */
function comment_entity_info() {
  $return =  array(
    'comment' => array(
      'label' => t('Comment'),
      'base table' => 'comment',
      'fieldable' => TRUE,
      'controller class' => 'CommentController',
      'object keys' => array(
        'id' => 'cid',
        'bundle' => 'node_type',
      ),
      'path' => 'comment',
      'rdf-type' => array('sioc:Post'),
      'bundle keys' => array(
        'bundle' => 'type',
      ),
      'bundles' => array(),
      'static cache' => FALSE,
      'name property' => 'title',
    ),
  );

  foreach (node_type_get_names() as $type => $name) {
    $return['comment']['bundles']['comment_node_' . $type] = array(
      'label' => $name,
    );
  }

  // Add meta-data about the basic comment properties.
  $properties = &$return['comment']['properties'];

  $properties['cid'] = array(
    'label' => t("Comment ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the comment."),
  );
  $properties['pid'] = array(
    'label' => t("Parent ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the comment's parent, if comment threading is active."),
  );
  $properties['nid'] = array(
    'label' => t("Node ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node the comment was posted to."),
  );
  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the user who posted the comment."),
  );
  $properties['hostname'] = array(
    'label' => t("IP Address"),
    'description' => t("The IP address of the computer the comment was posted from."),
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name left by the comment author."),
    'getter callback' => 'comment_get_properties',
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['mail'] = array(
    'label' => t("Email address"),
    'description' => t("The email address left by the comment author."),
    'getter callback' => 'comment_get_properties',
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['homepage'] = array(
    'label' => t("Home page"),
    'description' => t("The home page URL left by the comment author."),
    'sanitize' => 'filter_xss_bad_protocol',
    'setter callback' => 'drupal_property_verbatim_set',
  );
  $properties['title'] = array(
    'label' => t("Title"),
    'description' => t("The title of the comment."),
    'getter callback' => 'comment_get_properties',
    'rdf-property' => array('dc:title'),
  );
  $properties['body'] = array(
    'label' => t("Content"),
    'description' => t("The formatted content of the comment itself."),
    'getter callback' => 'comment_get_properties',
    'setter callback' => 'drupal_property_verbatim_set',
    'rdf-property' => array('content:encoded'),
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the comment."),
    'getter callback' => 'comment_get_properties',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The URL of the comment's edit page."),
    'getter callback' => 'comment_get_properties',
  );
  $properties['created'] = array(
    'label' => t("Date created"),
    'description' => t("The date the comment was posted."),
    'type' => 'date',
    'setter callback' => 'drupal_property_verbatim_set',
    'rdf-property' => array('dc:created'),
  );
  $properties['parent'] = array(
    'label' => t("Parent"),
    'description' => t("The comment's parent, if comment threading is active."),
    'type' => 'comment',
    'getter callback' => 'comment_get_properties',
    'rdf-property' => array('sioc:reply_of'),
  );
  $properties['node'] = array(
    'label' => t("Node"),
    'description' => t("The node the comment was posted to."),
    'type' => 'node',
    'getter callback' => 'comment_get_properties',
    'rdf-property' => array('sioc:reply_of'),
  );
  $properties['author'] = array(
    'label' => t("Author"),
    'description' => t("The author of the comment, if they were logged in."),
    'type' => 'user',
    'getter callback' => 'comment_get_properties',
    'rdf-property' => array('sioc:has_creator'),
  );
  $properties['rdf-uri'] = array(
    'label' => t("RDF URI"),
    'description' => t("The RDF URI of this entity."),
    'getter callback' => 'comment_get_properties',
  );
  return $return;
}

/**
 * Implement hook_entity_info_alter().
 */
function comment_entity_info_alter(&$entity_info) {
  $properties = &$entity_info['node']['properties'];

  $properties['comment-count'] = array(
    'label' => t("Comment count"),
    'description' => t("The number of comments posted on a node."),
    'getter callback' => 'comment_get_node_properties',
  );
  $properties['comment-count-new'] = array(
    'label' => t("New comment count"),
    'description' => t("The number of comments posted on a node since the reader last viewed it."),
    'getter callback' => 'comment_get_node_properties',
  );
}
