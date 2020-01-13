<?php
if (!function_exists('aggregate')) {
    /**
     * Aggregate it!
     *
     * @param string $input
     * @return string
     */
    function aggregate($input)
    {
        return (new \CIDRAM\Aggregator\Aggregator())->aggregate($input);
    }
}
