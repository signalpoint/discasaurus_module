<?php

use \Drupal\Core\Link;
use \Drupal\Core\Url;

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

  // START HEAVING LIFTING...

  $nid = $options['nid'];

  switch ($type) {

    // Courses.
    case 'dg_course':

      $d6_course = d6_node_load($nid, $type);

      $message = 'Migrating ' . $d6_course['title'];
      $context['results'][] = $d6_course['nid'];

      $d8_course = d6_node_to_d8($d6_course);
      //$d8_course->save();

      break;
    
    // Holes.
    case 'dg_hole':

      $d6_hole = d6_node_load($nid, $type);

      $message = 'Migrating ' . $d6_hole['title'];
      $context['results'][] = array(
        'nid' => $d6_hole['nid'],
        'type' => $type,
        'page' => $options['page']
      );

      $d8_hole = d6_node_to_d8($d6_hole);
      $d8_hole->save();
      
      break;

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
    dpm($results[0]);
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message(
    t($message . ': %try : %nextPage', array(
      '%try' => Link::createFromRoute(
        'Try again',
        'disc.batch'
      )->toString(),
      '%nextPage' => Link::createFromRoute(
        'Next page',
        'disc.batch',
        ['type' => $results[0]['type'], 'page' => $results[0]['page'] + 1]
      )->toString()
    ))
  );
  //$_SESSION['disc_migrate_batch_results'] = $foo;
}
