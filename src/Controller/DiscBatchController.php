<?php

/**
 * @file
 * Contains \Drupal\disc\Controller\DiscBatchController.
 */

namespace Drupal\disc\Controller;

use \Drupal\Core\Link;
use \Drupal\Core\Url;

class DiscBatchController {
  public function content() {

    $type = $_GET['type'];
    $operations = [];

    switch ($type) {

      // Course locations.
      case 'dg_course_location':


        disc_switch_db(6);
        $result = db_select('node', 'n')
          ->fields('n', array('nid', 'title'))
          ->condition('n.type', $type)
          ->orderBy('n.nid', 'ASC')
          ->range(0, 10)
          ->execute();
        disc_switch_db(8);
        disc_switch_db(8);
        foreach ($result as $item) {
          $operations[] = array(
            'disc_migrate',
            array('dg_course_location', array(
              'nid' => $result->nid,
              'title' => $result->title
            ))
          );
        }
        break;

      // Migration type chooser.
      default:
        return array(
          '#theme' => 'item_list',
          '#items' => array(
            Link::createFromRoute('Course locations',
              'disc.batch',
              ['type' => 'dg_course_location']
            )
          )
        );
        break;
    }

    if (empty($operations)) { return array('#markup' => 'No operations: ' . $type); }
    $batch = array(
      'title' => t('Exporting'),
      'operations' => $operations,
      'finished' => 'disc_migrate_finished_callback',
      'file' => drupal_get_path('module', 'disc') . '/disc.migrate.inc',
    );
    batch_set($batch);
    return batch_process('user');
  }
}