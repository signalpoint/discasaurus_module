<?php

use \Drupal\Core\Link;
use \Drupal\Core\Url;
use \Drupal\node\Entity\Node;

function disc_migrate($type, $options = array(), &$context) {

  // The $context array gathers batch context information about the execution (read),
  // as well as 'return values' for the current operation (write)
  // The following keys are provided :
  // 'results' (read / write): The array of results gathered so far by
  //   the batch processing, for the current operation to append its own.
  // 'message' (write): A text message displayed in the progress page.
  // The following keys allow for multi-step operations :
  // 'sandbox' (read / write): An array that can be freely used to
  //   store persistent data between iterations. It is recommended to
  //   use this instead of $_SESSION, which is unsafe if the user
  //   continues browsing in a separate window while the batch is processing.
  // 'finished' (write): A float number between 0 and 1 informing
  //   the processing engine of the completion level for the operation.
  //   1 (or no value explicitly set) means the operation is finished
  //   and the batch processing can continue to the next operation.

  $message = 'Ready spaghetti...';

  // START HEAVY LIFTING...

  $save = TRUE; // Set to true to start saving nodes, false for debugging.

  switch ($type) {

    // Courses.
    case 'dg_course':

      $nid = $options['nid'];

      // Load the Drupal 6 course node, convert it to a Drupal 8 course node and
      // save the new Drupal 8 node.
      $d6_course = d6_node_load($nid, $type);
      //dpm($d6_course);
      $message = 'Migrating ' . $d6_course['title'];
      $context['results'][] = $d6_course['nid'];

      // Convert it to a Drupal 8 node.
      $d8_course = d6_node_to_d8($d6_course);

      // Course location images.
      $images = d6_course_location_images(
        $d6_course['field_course_location_nid'],
        $d6_course['field_course_location_vid']
      );
      unset($d6_course['field_course_location_nid']);
      unset($d6_course['field_course_location_vid']);
      if (!empty($images)) {
        $d8_course->field_images = array();
        foreach ($images as $image) {
          $data = file_get_contents('http://www.discasaurus.com/' . $image->filepath);
          $directoryPath = str_replace('/' . $image->filename, '', $image->filepath);
          if (!file_exists($directoryPath)) {
            if (!mkdir($directoryPath, 0775, true)) {
              drupal_set_message('Failed to create directory: ' . $directoryPath, 'error');
            }
          }
          $filePath = 'public://' . str_replace('sites/default/files/', '', $image->filepath);
          $file = file_save_data($data, $filePath, FILE_EXISTS_REPLACE);
          if ($file) {
            $d8_course->field_images[] = array(
              'target_id' => $file->id(),
              'alt' => isset($image->field_dg_course_location_images_data) ?
                $image->field_dg_course_location_images_data['description'] : ''
            );
          }
        }
      }

      // Finally save the course node.
      if ($save) { $d8_course->save(); }

      // Along the way, store a mapping of D6 hole nids to their respective D8
      // nids, that way we don't have to constantly look it up in the DB.
      $hole_nodes = array();

      // Load all the Drupal 6 hole nodes associated with this course.
      $d6_holes = d6_course_holes($nid);
      //dpm($d6_holes);
      if (!isset($d6_holes) || empty($d6_holes)) { break; }
      foreach($d6_holes as $d6_hole_nid) {
        $d6_hole = d6_node_load($d6_hole_nid, 'dg_hole');
        //dpm($d6_hole);
        $d8_hole = d6_node_to_d8($d6_hole);

        // Hole images.
        $images = d6_hole_images($d6_hole['nid'], $d6_hole['vid']);
        unset($d6_hole['field_hole_nid']);
        unset($d6_hole['field_hole_vid']);
        if (!empty($images)) {
          $d8_hole->field_images = array();
          foreach ($images as $image) {
            $data = file_get_contents('http://www.discasaurus.com/' . $image->filepath);
            $directoryPath = str_replace('/' . $image->filename, '', $image->filepath);
            if (!file_exists($directoryPath)) {
              if (!mkdir($directoryPath, 0775, true)) {
                drupal_set_message('Failed to create directory: ' . $directoryPath, 'error');
              }
            }
            $filePath = 'public://' . str_replace('sites/default/files/', '', $image->filepath);
            $file = file_save_data($data, $filePath, FILE_EXISTS_REPLACE);
            if ($file) {
              $d8_hole->field_images[] = array(
                'target_id' => $file->id(),
                'alt' => isset($image->field_dg_hole_images_data['data']) ?
                  $image->field_dg_hole_images_data['data'] : ''
              );
            }
          }
        }

        // Finally save the hole node.
        if ($save) {
          $d8_hole->save();
          $hole_nodes[$d6_hole_nid] = $d8_hole->id();
        }

      }

      // Load all the Drupal 6 layouts nodes associated with this course.
      $d6_layouts = d6_course_layouts($nid);
      //dpm($d6_layouts);
      if (!isset($d6_layouts) || empty($d6_layouts)) { break; }
      foreach($d6_layouts as $d6_layout_nid) {
        $d6_layout = d6_node_load($d6_layout_nid, 'dg_layout');

        // Explode all the target ids on the layout's holes entity reference
        // field.
        $holes = explode(',', $d6_layout['field_dg_layout_hole_config_value']);
        unset($d6_layout['field_dg_layout_hole_config_value']);
        if (!empty($holes)) {
          $d6_layout['field_holes'] = array();
          foreach($holes as $d6_hole_nid) {
            $d6_layout['field_holes'][] = array('target_id' => $hole_nodes[$d6_hole_nid]);
          }
        }
        //dpm($d6_layout);

        $d8_layout = d6_node_to_d8($d6_layout);
        if ($save) { $d8_layout->save(); }
      }

      break;

    case 'user':

      $uid = $options['uid'];

      $d6_user = d6_user_load($uid);
      //dpm($d6_user);
      
      $message = 'Migrating ' . $d6_user['name'] . "({$d6_user['uid']})";
      $context['results'][] = $d6_user['uid'];

      $d8_user = d6_user_to_d8($d6_user);

      // User image.
      if (isset($d6_user['profile']['profile']['picture']) && !empty($d6_user['profile']['profile']['picture'])) {
        $image = $d6_user['profile']['profile']['picture'];
        $d8_user->user_picture = array();
        $data = file_get_contents('http://www.discasaurus.com/' . str_replace(' ', '%20', $image['filepath']));
        $directoryPath = str_replace('/' . $image['filename'], '', $image['filepath']);
        if (!file_exists($directoryPath)) {
          if (!mkdir($directoryPath, 0775, true)) {
            drupal_set_message('Failed to create directory: ' . $directoryPath, 'error');
          }
        }
        $filePath = 'public://' . str_replace('sites/default/files/', '', $image['filepath']);
        $file = file_save_data($data, $filePath, FILE_EXISTS_REPLACE);
        if ($file) {
          $d8_user->user_picture[] = array(
            'target_id' => $file->id()
          );
        }
      }

      // User favorite courses.

      
      if ($save) { $d8_user->save(); }

      break;

//    case 'dg_score':
//      // When importing a score check for other scores posted within a few
//      // seconds of each other at the same course, and place them all together
//      // on a round.
//      break;

    // WTF.
    default:
      $message = 'migrating wtf...';
      break;
  }

  $context['message'] = $message;
}

function disc_migrate_finished_callback($success, $results, $operations) {
  // The 'success' parameter means no fatal PHP errors were detected. All
  // other error management should be handled using 'results'.
  if ($success) {
    $message = \Drupal::translation()->formatPlural(
      count($results),
      'One post processed.',
      '@count posts processed.'
    );
  }
  else {
    $message = t('Finished with an error.');
  }
  $nextPageQueryString = array();
  if (isset($results[0]['type'])) { $nextPageQueryString['type'] = $results[0]['type']; }
  if (isset($results[0]['page'])) { $nextPageQueryString['page'] = $results[0]['page'] + 1; }
  drupal_set_message(
    t($message . ': %try : %nextPage', array(
      '%try' => Link::createFromRoute(
        'Try again',
        'disc.batch'
      )->toString(),
      '%nextPage' => Link::createFromRoute(
        'Next page',
        'disc.batch',
        $nextPageQueryString
      )->toString()
    ))
  );
  //$_SESSION['disc_migrate_batch_results'] = $foo;
}
