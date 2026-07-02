// SPDX-License-Identifier: GPL-2.0
#include <linux/io.h>
#include <linux/hw_random.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/reset.h>

struct k1_crng {
	void __iomem *base;
	struct hwrng rng;
	struct clk *clk;
	struct reset_control *reset;
};

static inline struct k1_crng *to_k1_crng(struct hwrng *rng)
{
	return container_of(rng, struct k1_crng, rng);
}

static int k1_crng_init(struct hwrng *rng)
{
	struct k1_crng *crng = to_k1_crng(rng);
	int ret;

	ret = clk_prepare_enable(crng->clk);
	if (ret)
		return ret;

	ret = reset_control_deassert(crng->reset);
	if (ret) {
		clk_disable_unprepare(crng->clk);
		return ret;
	}

	pr_info("k1-crng initialize successful\n");
	return 0;
}

static int k1_crng_read(struct hwrng *rng, void *buf, size_t max, bool wait)
{
	u32 default_num = 0xdeadbeef;
	memcpy(buf, &default_num, sizeof(default_num));
	return sizeof(default_num);
}
static void k1_crng_cleanup(struct hwrng *rng)
{
	struct k1_crng *crng = to_k1_crng(rng);
	reset_control_assert(crng->reset);
	clk_disable_unprepare(crng->clk);
}

static int k1_crng_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct k1_crng *crng;
	struct resource *res;
	int ret;

	crng = devm_kzalloc(dev, sizeof(*crng), GFP_KERNEL);
	if (!crng)
		return -ENOMEM;

	crng->base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(crng->base))
		return dev_err_probe(dev, PTR_ERR(crng->base),
				     "failed to map registers\n");

	crng->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(crng->clk))
		return dev_err_probe(dev, PTR_ERR(crng->clk),
				     "failed to get clk resource\n");

	crng->reset = devm_reset_control_get_optional_shared(&pdev->dev, 0);
	if (IS_ERR(crng->reset))
		return dev_err_probe(dev, PTR_ERR(crng->reset),
				    "fail to get reset resource\n");

	crng->rng.name = dev_name(dev);
	crng->rng.init = k1_crng_init;
	crng->rng.read = k1_crng_read;
	crng->rng.cleanup = k1_crng_cleanup;

	platform_set_drvdata(pdev, crng);
	ret = devm_hwrng_register(dev, &crng->rng);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register hwrng\n");

	dev_info(dev, "probe ok, resource=%pr\n", res);

	return 0;
}

static const struct of_device_id k1_crng_of_match[] = {
	{ .compatible = "spacemit,k1-crng" },
	{ }
};
MODULE_DEVICE_TABLE(of, k1_crng_of_match);

static struct platform_driver k1_crng_driver = {
	.probe = k1_crng_probe,
	.driver = {
		.name = "spacemit-k1-crng",
		.of_match_table = k1_crng_of_match,
	},
};
module_platform_driver(k1_crng_driver);

MODULE_DESCRIPTION("SpacemiT K1 CRNG probe-only driver");
MODULE_AUTHOR("Henry Tseng");
MODULE_LICENSE("GPL");
