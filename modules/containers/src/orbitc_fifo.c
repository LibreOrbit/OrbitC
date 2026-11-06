/*
 * ================================================================================
 * File: modules/containers/src/orbitc_fifo.c
 * ================================================================================
 */
#include "orbitc_fifo.h"

#if (ENABLE_CONTAINER_FIFO == 1)

#define ORBITC_FIFO_MAGIC ((ui32_t)0x4649464FUL) /* "FIFO" */

static inline void orbitc_fifo_copy(byte_t *dst, const byte_t *src, usize_t len)
{
    usize_t i;
    for (i = 0U; i < len; ++i)
    {
        dst[i] = src[i];
    }
}

status_t orbitc_fifo_init(fifo_t *fifo, void *buf, usize_t elem_size, usize_t capacity)
{
    status_t st = STATUS_OK;

    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(buf, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (buf == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }

    if ((elem_size == 0U) || (capacity == 0U) || ((ORBITC_USIZE_MAX / elem_size) < capacity))
    {
        orbitc_assert_param(ORBITC_FALSE, &st, STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_INVALID_PARAM);
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_INVALID_PARAM);
    }

    fifo->buf        = (byte_t *)buf;
    fifo->elem_size  = elem_size;
    fifo->capacity   = capacity;
    fifo->head       = 0U;
    fifo->tail       = 0U;
    fifo->count      = 0U;
    fifo->high_water = 0U;
    fifo->init_magic = ORBITC_FIFO_MAGIC;

    return st;
}

status_t orbitc_fifo_reset(fifo_t *fifo)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if (fifo == ORBITC_NULL)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }

    fifo->head  = 0U;
    fifo->tail  = 0U;
    fifo->count = 0U;
    return st;
}

status_t orbitc_fifo_push(fifo_t *fifo, const void *elem)
{
    status_t st = STATUS_OK;
    byte_t  *dst;

    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (elem == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    if (fifo->count == fifo->capacity)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_CAPACITY, CONTAINERS_CODE_FULL);
    }

    dst = fifo->buf + (fifo->head * fifo->elem_size);
    orbitc_fifo_copy(dst, (const byte_t *)elem, fifo->elem_size);

    fifo->head  = (fifo->head + 1U) % fifo->capacity;
    fifo->count += 1U;
    if (fifo->count > fifo->high_water)
    {
        fifo->high_water = fifo->count;
    }

    return st;
}

status_t orbitc_fifo_pop(fifo_t *fifo, void *out_elem)
{
    status_t      st = STATUS_OK;
    const byte_t *src;

    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (out_elem == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    if (fifo->count == 0U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NOT_FOUND, CONTAINERS_CODE_EMPTY);
    }

    src = fifo->buf + (fifo->tail * fifo->elem_size);
    orbitc_fifo_copy((byte_t *)out_elem, src, fifo->elem_size);

    fifo->tail  = (fifo->tail + 1U) % fifo->capacity;
    fifo->count -= 1U;
    return st;
}

status_t orbitc_fifo_peek(const fifo_t *fifo, void *out_elem)
{
    status_t      st = STATUS_OK;
    const byte_t *src;

    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_elem, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (out_elem == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    if (fifo->count == 0U)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NOT_FOUND, CONTAINERS_CODE_EMPTY);
    }

    src = fifo->buf + (fifo->tail * fifo->elem_size);
    orbitc_fifo_copy((byte_t *)out_elem, src, fifo->elem_size);
    return st;
}

status_t orbitc_fifo_is_empty(const fifo_t *fifo, bool_t *out_empty)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_empty, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (out_empty == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    *out_empty = (fifo->count == 0U) ? ORBITC_TRUE : ORBITC_FALSE;
    return st;
}

status_t orbitc_fifo_is_full(const fifo_t *fifo, bool_t *out_full)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_full, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (out_full == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    *out_full = (fifo->count == fifo->capacity) ? ORBITC_TRUE : ORBITC_FALSE;
    return st;
}

status_t orbitc_fifo_get_count(const fifo_t *fifo, usize_t *out_count)
{
    status_t st = STATUS_OK;
    orbitc_assert_not_null(fifo, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    orbitc_assert_not_null(out_count, &st, STATUS_MOD_CONTAINERS, (ui8_t)CONTAINERS_CODE_NULL_PTR);
    if ((fifo == ORBITC_NULL) || (out_count == ORBITC_NULL))
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_NULL_PTR, CONTAINERS_CODE_NULL_PTR);
    }
    if (fifo->init_magic != ORBITC_FIFO_MAGIC)
    {
        return STATUS_MAKE(STATUS_SEV_ERROR, STATUS_MOD_CONTAINERS, STATUS_CLS_INVALID_PARAM, CONTAINERS_CODE_NOT_INIT);
    }
    *out_count = fifo->count;
    return st;
}

#else
typedef int orbitc_fifo_disabled_tu_t;
#endif /* ENABLE_CONTAINER_FIFO == 1 */