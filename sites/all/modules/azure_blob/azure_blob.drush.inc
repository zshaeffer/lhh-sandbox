<?php

/**
 * @file
 * Migrate files between storage engines.
 * 
 * @author Damien Tournoud (Windows Azure Integration Developer), atuyo60
 */

/**
 * Implements hook_drush_help().
 * 
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'. This hook is optional. If a command
 * does not implement this hook, the command's description is used instead.
 * 
 * @param string $command
 *   A string with the help section (prepend with 'drush:')
 * 
 * @return string
 *   A string with the help text for your command.
 */
function azure_blob_drush_help($command) {
  switch ($command) {
    case 'drush:azure-blob-migrate':
      return dt('Migrate files between storage engines.');
  }
}

/**
 * Implements hook_drush_command().
 * 
 * Declares a new migration command for Windows Azure.
 * 
 * @return array
 *   An array of new Drush commands.
 */
function azure_blob_drush_command() {
  $items = array();
  
  $items['azure-blob-migrate'] = array(
    'description' => dt('Migrate files between storage engines'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'core' => array(7),
    'arguments' => array(
      'source' => dt('The name of the source storage (for example: public, private, azure, etc.)'),
      'destination' => dt('The name of the destination storage'),
    ),
    'aliases' => array('azbm'),
  );
  
  return $items;
}

/**
 * Drush command callback.
 * 
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 * 
 * @param string $source_storage
 * @param string $destination_storage
 */
function drush_azure_blob_azure_blob_migrate($source_storage = NULL, $destination_storage = NULL) {
  if (!isset($source_storage)) {
    return drush_set_error('DRUSH_AZURE_BLOB_NO_SOURCE', dt('Missing argument: source'));
  }
  if (!isset($destination_storage)) {
    return drush_set_error('DRUSH_AZURE_BLOB_NO_DESTINATION', dt('Missing argument: destination'));
  }

  $query = db_select('file_managed', 'f')
    ->fields('f')
    ->condition('f.uri', $source_storage . '://%', 'LIKE');

  // Fetch the total number of files for feedback.
  $total = $query->countQuery()->execute()->fetchField();

  $last_feedback = time();
  $count = 0;
  
  foreach ($query->execute() as $file) {
    $destination_uri = $destination_storage . '://' . file_uri_target($file->uri);
    file_move($file, $destination_uri, FILE_EXISTS_REPLACE);

    ++$count;
    
    // Log progress every 10 seconds
    if (time() - $last_feedback >= 10) {
      drush_log(dt('Migrated !count files out of !total.', array('!count' => $count, '!total' => $total)), 'ok');
      $last_feedback = time();
    }
  }

  drush_log(dt('Migrated !count files.', array('!count' => $count)), 'ok');
}
