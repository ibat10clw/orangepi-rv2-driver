// SPDX-License-Identifier: GPL-2.0
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

struct k1_crng {
	void __iomem *base;
};

static int k1_crng_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct k1_crng *crng;
	struct resource *res;

	crng = devm_kzalloc(dev, sizeof(*crng), GFP_KERNEL);
	if (!crng)
		return -ENOMEM;

	crng->base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(crng->base))
		return dev_err_probe(dev, PTR_ERR(crng->base),
				     "failed to map registers\n");

	platform_set_drvdata(pdev, crng);

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
