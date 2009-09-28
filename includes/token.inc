<?php
// $Id: token.inc,v 1.5 2009-08-25 02:50:08 webchick Exp $

/**
 * @file
 * Drupal placeholder/token replacement system.
 *
 * Provides a set of extensible API functions for replacing placeholders in text
 * with meaningful values.
 *
 * For example: When configuring automated emails, an administrator enters standard
 * text for the email. Variables like the title of a node and the date the email
 * was sent can be entered as placeholders like [node:title] and [date:short].
 * When a Drupal module prepares to send the email, it can call the token_replace()
 * function, passing in the text. The token system will scan the text for placeholder
 * tokens, give other modules an opportunity to replace them with meaningful text,
 * then return the final product to the original module.
 *
 * Tokens follow the form: [$type:$name], where $type is a general class of
 * tokens like 'node', 'user', or 'comment' and $name is the name of a given
 * placeholder. For example, [node:title].
 *
 * In addition to raw text containing placeholders, modules may pass in an array
 * of objects to be used when performing the replacement. The objects should be
 * keyed by the token type they correspond to. For example:
 *
 * @code
 * // Load a node and a user, then replace tokens in the text.
 * $text = 'On [date:short], [user:name] read [node:title].';
 * $node = node_load(1);
 * $user = user_load(1);
 *
 * // [date:...] tokens use the current date automatically.
 * $data = array('node' => $node, 'user' => $user);
 * return token_replace($text, $data);
 * @endcode
 *
 * Some tokens may be chained in the form of [$type:$pointer:$name], where $type
 * is a normal token type, $pointer is a reference to another token type, and
 * $name is the name of a given placeholder. For example, [node:author:mail]. In
 * that example, 'author' is a pointer to the 'user' account that created the node,
 * and 'mail' is a placeholder available for any 'user'.
 *
 * @see token_replace()
 * @see hook_token_format_info()
 * @see hook_entity_info()
 */

/**
 * Replace all tokens in a given string with appropriate values.
 *
 * @param $text
 *   A string potentially containing replacable tokens.
 * @param $data
 *   (optional) An array of keyed objects. For simple replacement scenarios
 *   'node', 'user', and others are common keys, with an accompanying node or
 *   user object being the value. Some token types, like 'system', do not require
 *   any explicit information from $data and can be replaced even if it is empty.
 * @param $options
 *   (optional) A keyed array of settings and flags to control the token
 *   replacement process. Supported options are:
 *   - language: A language object to be used when generating locale-sensitive
 *     tokens.
 *   - callback: A callback function that will be used to post-process the array
 *     of token replacements after they are generated. For example, a module using
 *     tokens in a text-only email might provide a callback to strip HTML
 *     entities from token values before they are inserted into the final text.
 *   - clear: A boolean flag indicating that tokens should be removed from the
 *     final text if no replacement value can be generated. Defaults to TRUE.
 *   - sanitize: A boolean flag indicating that tokens should be sanitized for
 *     display to a web browser. Defaults to TRUE. Developers who set this option
 *     to FALSE assume responsibility for running filter_xss(), check_plain() or
 *     other appropriate scrubbing functions before displaying data to users.
 * @param $types
 *   (optional) An array assigning types to the objects of $data, keyed liked
 *   $data. If there is no assigned type for a data object, the key specified in
 *   $data is used as type.
 * @return
 *   Text with tokens replaced.
 */
function token_replace($text, array $data = array(), array $options = array(), array $types = array()) {
  $token_list = token_scan($text);
  $replacements = token_generate($token_list, $data, $options, $types);

  // Optionally alter the list of replacement values.
  if (!empty($options['callback']) && function_exists($options['callback'])) {
    $function = $options['callback'];
    $function($replacements, $data, $options);
  }

  $tokens = array_keys($replacements);
  $values = array_values($replacements);

  return str_replace($tokens, $values, $text);
}

/**
 * Build a list of all token-like patterns that appear in the text.
 *
 * @param $text
 *   The text to be scanned for possible tokens.
 * @return
 *   An associative array of discovered tokens, grouped by type.
 */
function token_scan($text) {
  // Matches tokens with the following pattern: [$type:$token]
  // $type and $token may not contain white spaces.
  preg_match_all('/\[([^\s\]:]*):([^\s\]]*)\]/', $text, $matches);

  $types = $matches[1];
  $tokens = $matches[2];

  // Iterate through the matches, building an associative array containing
  // $tokens grouped by $types, pointing to the version of the token found in
  // the source text. For example, $results['node']['title'] = '[node:title]';
  $results = array();
  for ($i = 0; $i < count($tokens); $i++) {
    $results[$types[$i]][$tokens[$i]] = $matches[0][$i];
  }

  return $results;
}

/**
 * Generate replacement values for a list of tokens.
 *
 * @param $raw_tokens
 *   A keyed array of tokens, and their original raw form in the source text.
 * @param $data
 *   (optional) An array of keyed objects. For simple replacement scenarios
 *   'node', 'user', and others are common keys, with an accompanying node or
 *   user object being the value. Some token types, like 'system', do not require
 *   any explicit information from $data and can be replaced even if it is empty.
 * @param $options
 *   (optional) A keyed array of settings and flags to control the token
 *   replacement process. Supported options are:
 *   - 'language' A language object to be used when generating locale-sensitive
 *     tokens.
 *   - 'callback' A callback function that will be used to post-process the array
 *     of token replacements after they are generated. Can be used when modules
 *     require special formatting of token text, for example URL encoding or
 *     truncation to a specific length.
 *   - 'sanitize' A boolean flag indicating that tokens should be sanitized for
 *     display to a web browser. Developers who set this option to FALSE assume
 *     responsibility for running filter_xss(), check_plain() or other
 *     appropriate scrubbing functions before displaying data to users.
 *   - clear: A boolean flag indicating that tokens should be removed from the
 *     final text if no replacement value can be generated. Defaults to TRUE.
 * @param $types
 *   (optional) An array assigning types to the objects of $data, keyed liked
 *   $data. If there is no assigned type for a data object, the key specified in
 *   $data is used as type.
 * @return
 *   An associative array of replacement values, keyed by the original 'raw'
 *   tokens that were found in the source text. For example:
 *   $results['[node:title]'] = 'My new node';
 */
function token_generate(array $raw_tokens, array $data = array(), array $options = array(), array $types = array()) {
  $results = array();
  $options += array('sanitize' => TRUE, 'clear' => TRUE);
  // Add in the current date and the global user by default.
  $data += array('date' => REQUEST_TIME, 'current-user' => $GLOBALS['user']);
  $types += array('current-user' => 'user');

  foreach ($raw_tokens as $key => $tokens) {
    $types += array($key => $key);

    if (isset($data[$key]) && ($wrapper = drupal_get_property_wrapper($types[$key], $data[$key], $options))) {
      foreach ($tokens as $token => $original) {
        try {
          $results[$original] = _token_get_replacement($wrapper, $token);
        }
        catch (DrupalPropertyWrapperException $e) {
          // A token has not been found.
          if ($options['clear']) {
            $results[$original] = '';
          }
        }
      }
    }
  }
  return $results;
}

/**
 * Applies chained tokens by getting properties of the given wrapper.
 */
function _token_get_replacement(DrupalPropertyWrapperInterface $wrapper, $token) {
  foreach (explode(':', $token) as $i => $name) {
    $wrapper = $wrapper->$name;
  }
  // If no format was given apply the default just by converting it to string.
  return (string)$wrapper;
}


/**
 * Formats a token by using the formats defined in drupal_get_datatype_info().
 *
 * @param $value
 *   The token value to format.
 * @param $type
 *   The type of the passed token.
 * @param $format
 *   (optional) The format to apply. If unset, the default format will be used.
 *   If there is no default format either, the passed value is returned.
 * @param $options
 *   (optional) A keyed array of options. Supported are:
 *   - language: A language object to be used when generating locale-sensitive
 *     formats.
 *   - absolute: Whether generated URLs should be absolute. Defaults to TRUE.
 *   - formats: An array of additional formats to use.
 *   - default format: May be used to override the default format.
 * @param $context
 *   (optional) An array of contextual information which format callbacks
 *   can make use of. Used internally.
 */
function token_format($value, $type, $format = NULL, array $options = array(), array $context = array()) {
  $info = drupal_get_datatype_info($type) + array('formats' => array(), 'default format' => NULL);
  $options += array('language' => NULL, 'formats' => array(), 'absolute' => TRUE);
  $formats = $options['formats'] + $info['formats'];
  if (!isset($format)) {
    $format = isset($options['default format']) ? $options['default format'] : $info['default format'];
  }
  // Now apply the format.
  if (isset($formats[$format]['callback']) && function_exists($function = $formats[$format]['callback'])) {
    return $function($value, $format, $options, $context);
  }
  elseif (!isset($format)) {
    return $value;
  }
}
